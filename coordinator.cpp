#include "common.h"
#include "coordinator.h"
#include "logger.h"
#include <iostream>

using namespace std;

// Global lock table
map<int, bool> key_locks;
mutex lock_mutex;

bool is_key_locked(int key) {
    lock_guard guard(lock_mutex);
    return key_locks[key];
}

void lock_key(int key) {
    lock_guard guard(lock_mutex);
    key_locks[key] = true;
}

void unlock_key(int key) {
    lock_guard guard(lock_mutex);
    key_locks[key] = false;
}

void coordinator(const int &shardNodes) {
    auto& logger = Logger::getInstance();

    logger.info("Coordinator started...\n", 0);

    while (true) {
        auto [client_rank, Type, key, value] = ClientRequest::receive_client_request(MPI_ANY_SOURCE, CLIENT_REQUEST, MPI_COMM_WORLD);
        const auto type = selectType(Type);

        if (type == READ) {
            vector<ShardResponse> responses;

            for (int shard_id = 1; shard_id <= shardNodes; ++shard_id) {
                ShardRequest read_request{READ, key, ""};
                ShardRequest::send_shard_request(read_request, shard_id, NODE_REQUEST, MPI_COMM_WORLD);

                auto response = ShardResponse::receive_shard_response(shard_id, NODE_RESPONSE, MPI_COMM_WORLD);
                responses.push_back(response);
            }

            // Determine the latest version of the value
            string latest_value;
            bool found = false;

            for (auto &[success, value] : responses) {
                if (!found && success) {
                    latest_value = value;
                    found = true;
                }
            }

            if (found) {
                ClientRequest::send_client_response({true, latest_value}, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
            } else {
                ClientRequest::send_client_response({false, "Key not found"}, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
            }
        } else {
            if (is_key_locked(key)) {
                logger.warning("Key " + to_string(key) + " is already locked, rejecting request", 0);
                ClientRequest::send_client_response(
                    {false, "Key is locked by another transaction"},
                    client_rank,
                    CLIENT_RESPONSE,
                    MPI_COMM_WORLD
                );
                continue;
            }

            lock_key(key);
            logger.debug("Locked key: " + to_string(key), 0);

            // WRITE operations require two-phase commit
            bool prepare_success = true;

            // Send PREPARE requests to all shards
            for (int shard_id = 1; shard_id <= shardNodes; ++shard_id) {
                ShardRequest shard_request{type, key, value, PREPARE};
                ShardRequest::send_shard_request(shard_request, shard_id, NODE_REQUEST, MPI_COMM_WORLD);
            }

            // Collect PREPARE responses from all shards
            for (int shard_id = 1; shard_id <= shardNodes; ++shard_id) {
                const auto [success, str] = ShardResponse::receive_shard_response(shard_id, NODE_RESPONSE, MPI_COMM_WORLD);
                logger.info("Received from shard " + to_string(shard_id) + ": " + str, 0);
                prepare_success = prepare_success && success;
            }

            logger.info("Prepare phase success = " + string(prepare_success ? "true" : "false"), 0);

            // Decide on COMMIT or ROLLBACK based on PREPARE responses
            const TwoPC next_phase = prepare_success ? COMMIT : ROLLBACK;
            logger.info("Starting " + string(next_phase == COMMIT ? "COMMIT" : "ROLLBACK") + " phase", 0);

            // Send COMMIT or ROLLBACK to all shards
            for (int shard_id = 1; shard_id <= shardNodes; ++shard_id) {
                ShardRequest shard_request{type, key, value, next_phase};
                ShardRequest::send_shard_request(shard_request, shard_id, NODE_REQUEST, MPI_COMM_WORLD);
                // Getting the Acknowledgment
                ShardResponse::receive_shard_response(shard_id, NODE_RESPONSE, MPI_COMM_WORLD);
            }

            unlock_key(key);
            logger.info("Starting " + string(next_phase == COMMIT ? "COMMIT" : "ROLLBACK") + " phase", 0);

            ClientRequest::send_client_response(
                {prepare_success, prepare_success ? value : "Operation failed"},
                client_rank,
                CLIENT_RESPONSE,
                MPI_COMM_WORLD
            );
        }
    }
}


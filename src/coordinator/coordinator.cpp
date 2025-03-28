#include "../../include/common/types.h"
#include "../../include/common/logger.h"
#include "../../include/common/client_request_utils.h"
#include "../../include/common/client_response_utils.h"
#include "../../include/common/node_request_utils.h"
#include "../../include/common/node_response_utils.h"
#include "mpi.h"
#include <fstream>

using namespace std;

// Global lock table
map<int, bool> key_locks;
mutex lock_mutex;

bool is_key_locked(const int key) {
    lock_guard guard(lock_mutex);
    return key_locks[key];
}

void lock_key(const int key) {
    lock_guard guard(lock_mutex);
    key_locks[key] = true;
}

void unlock_key(const int key) {
    lock_guard guard(lock_mutex);
    key_locks[key] = false;
}


void coordinator(const int &nodes) {
    auto &logger = Logger::getInstance();
    logger.info("Coordinator started...\n", 0);

    while (true) {
        auto [client_rank, type, key, value] = receive_client_request(
            MPI_ANY_SOURCE, CLIENT_REQUEST, MPI_COMM_WORLD);

        if (type == RequestType::READ) {
            vector<NodeResponse> responses;

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                NodeRequest read_request{RequestType::READ, key, ""};
                send_node_request(read_request, node_id, NODE_REQUEST, MPI_COMM_WORLD);

                auto response = receive_node_response(node_id, NODE_RESPONSE, MPI_COMM_WORLD);
                responses.push_back(response);
            }

            string latest_value;
            bool found = false;

            for (auto &[success, value]: responses) {
                if (!found && success) {
                    latest_value = value;
                    found = true;
                }
            }

            if (found) {
                send_client_response({true, latest_value}, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
            } else {
                send_client_response({false, "Key not found"}, client_rank, CLIENT_RESPONSE,
                                     MPI_COMM_WORLD);
            }
        } else {
            if (is_key_locked(key)) {
                logger.warning("Key " + to_string(key) + " is already locked, rejecting request", 0);
                send_client_response(
                    {false, "Key is locked by another transaction"},
                    client_rank,
                    CLIENT_RESPONSE,
                    MPI_COMM_WORLD
                );
                continue;
            }

            lock_key(key);
            logger.debug("Locked key: " + to_string(key), 0);

            bool prepare_success = true;

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                NodeRequest node_request{type, key, value, TwoPC::PREPARE};
                send_node_request(node_request, node_id, NODE_REQUEST, MPI_COMM_WORLD);
            }

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                const auto [success, str] = receive_node_response(node_id, NODE_RESPONSE, MPI_COMM_WORLD);
                logger.info("Received from node " + to_string(node_id) + ": " + str, 0);
                prepare_success = prepare_success && success;
            }

            logger.info("Prepare phase success = " + string(prepare_success ? "true" : "false"), 0);

            const TwoPC next_phase = prepare_success ? TwoPC::COMMIT : TwoPC::ROLLBACK;
            logger.info("Starting " + string(next_phase == TwoPC::COMMIT ? "COMMIT" : "ROLLBACK") + " phase", 0);

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                NodeRequest node_request{type, key, value, next_phase};
                send_node_request(node_request, node_id, NODE_REQUEST, MPI_COMM_WORLD);
                receive_node_response(node_id, NODE_RESPONSE, MPI_COMM_WORLD);
            }

            unlock_key(key);
            logger.info("Starting " + string(next_phase == TwoPC::COMMIT ? "COMMIT" : "ROLLBACK") + " phase", 0);

            send_client_response(
                {prepare_success, prepare_success ? value : "Operation failed"},
                client_rank,
                CLIENT_RESPONSE,
                MPI_COMM_WORLD
            );
        }
    }
}

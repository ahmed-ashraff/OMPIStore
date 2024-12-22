#include "common.h"
#include "coordinator.h"
#include <iostream>

using namespace std;

void coordinator(int world_size) {
    cout << "Coordinator started...\n";

    while (true) {
        auto [client_rank, Type, key, value] = ClientRequest::receive_client_request(MPI_ANY_SOURCE, CLIENT_REQUEST, MPI_COMM_WORLD);
        const auto type = selectType(Type);

        if (type == READ) {
            vector<ShardResponse> responses;
            for (int shard_id = 1; shard_id < world_size; ++shard_id) {
                ShardRequest read_request{READ, key, ""};
                ShardRequest::send_shard_request(read_request, shard_id, SHARD_REQUEST, MPI_COMM_WORLD);

                auto response = ShardResponse::receive_shard_response(shard_id, SHARD_RESPONSE, MPI_COMM_WORLD);
                responses.push_back(response);
            }

            // Determine the latest version of the value
            string latest_value;
            bool found = false;
            for (const auto &[success, value] : responses) {
                if (success) {
                    latest_value = value;
                    found = true;
                    break;
                }
            }

            if (found) {
                cout << "Coordinator: Found value \"" << latest_value << "\" for key " << key << "\n";
                ClientRequest::send_client_response({true, latest_value}, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
            } else {
                cout << "Coordinator: Key " << key << " not found in any shard\n";
                ClientRequest::send_client_response({false, "Key not found"}, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
            }
        } else {
            // WRITE operations require two-phase commit
            bool prepare_success = true;

            // Send PREPARE requests to all shards
            for (int shard_id = 1; shard_id < world_size; ++shard_id) {
                ShardRequest shard_request{type, key, value, PREPARE};
                ShardRequest::send_shard_request(shard_request, shard_id, SHARD_REQUEST, MPI_COMM_WORLD);
            }

            // Collect PREPARE responses from all shards
            for (int shard_id = 1; shard_id < world_size; ++shard_id) {
                const auto [success, _] = ShardResponse::receive_shard_response(shard_id, SHARD_RESPONSE, MPI_COMM_WORLD);
                cout << "Coordinator received from shard " << shard_id << ": " << success << '\n';
                if (!success) {
                    prepare_success = false;
                    break;
                }
            }

            // Decide on COMMIT or ROLLBACK based on PREPARE responses
            const TransactionState next_phase = prepare_success ? COMMIT : ROLLBACK;

            // Send COMMIT or ROLLBACK to all shards
            for (int shard_id = 1; shard_id < world_size; ++shard_id) {
                ShardRequest shard_request{type, key, value, next_phase};
                ShardRequest::send_shard_request(shard_request, shard_id, SHARD_REQUEST, MPI_COMM_WORLD);
            }

            // Send response to client
            ClientRequest::send_client_response(
                {prepare_success, prepare_success ? value : "Operation failed"},
                client_rank,
                CLIENT_RESPONSE,
                MPI_COMM_WORLD
            );
        }
    }
}


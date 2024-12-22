#include "common.h"
#include "shard.h"
#include <iostream>
#include <map>
#include <string>

using namespace std;

void shard(const int &rank) {
    cout << "Shard " << rank << " started...\n";
    map<int, string> store;

    while (true) {
        auto [Type, key, value, recPhase] = ShardRequest::receive_shard_request(0, SHARD_REQUEST, MPI_COMM_WORLD);

        const auto phase = selectPhase(recPhase);
        const auto type = selectType(Type);

        ShardResponse shard_response{};

        if (type == READ) {
            // Respond to the READ request by checking the local store
            if (store.contains(key)) {
                cout << "Shard " << rank << ": Returning value \"" << store[key] << "\" for key " << key << "\n";
                shard_response = {true, store[key]}; // Return the value if the key exists
            } else {
                cout << "Shard " << rank << ": Key " << key << " not found\n";
                shard_response = {false, "Key not found"}; // Indicate that the key was not found
            }

            // Send the response back to the coordinator
            ShardResponse::send_shard_response(shard_response, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
            continue;
        }

        if (phase == PREPARE) {
            cout << "Shard " << rank << ": Received PREPARE request for key " << key << " Type: " << type << "\n";

            // Perform necessary checks/preparation for the operation
            if (type == CREATE && !store.contains(key)) {
                shard_response = {true, "PREPARE success: Key can be created"};
            } else if (type == UPDATE && store.contains(key)) {
                shard_response = {true, "PREPARE success: Key can be updated"};
            } else if (type == DELETE && store.contains(key)) {
                shard_response = {true, "PREPARE success: Key can be deleted"};
            } else {
                shard_response = {false, "PREPARE failed: Key constraints not satisfied"};
            }

            cout << "Shard " << rank << " sending response: " << shard_response.success << std::endl;
            // Send PREPARE response back to the coordinator
            ShardResponse::send_shard_response(shard_response, 0, SHARD_RESPONSE, MPI_COMM_WORLD);

            if (!shard_response.success) {
                continue;
            }
        }

        // Wait for the next phase (COMMIT or ROLLBACK)
        auto [next_type, next_key, next_value, Next_phase_int] = ShardRequest::receive_shard_request(0, SHARD_REQUEST, MPI_COMM_WORLD);
        const auto next_phase = selectPhase(Next_phase_int);

        // Ensure that the next request corresponds to the same key
        if (next_key != key) {
            cout << "Shard " << rank << ": Received mismatched key " << next_key << " for COMMIT/ROLLBACK. Waiting for next request...\n";
            continue; // Wait for the next request if keys don't match
        }

        if (next_phase == COMMIT) {
            cout << "Shard " << rank << ": Received COMMIT request for key " << next_key << "\n";

            // Execute the operation
            switch (type) {
                case CREATE:
                    store[key] = value;
                    shard_response = {true, value};
                    break;
                case UPDATE:
                    store[key] = value;
                    shard_response = {true, "UPDATED"};
                    break;
                case DELETE:
                    store.erase(key);
                    shard_response = {true, "Key deleted"};
                    break;
                default:
                    shard_response = {false, "Invalid operation"};
                    break;
            }
        } else if (next_phase == ROLLBACK) {
            cout << "Shard " << rank << ": Received ROLLBACK request for key " << next_key << "\n";
            shard_response = {true, "Operation rolled back"};
        } else {
            cout << "Shard " << rank << ": Invalid phase received\n";
            shard_response = {false, "Invalid phase"};
        }

        // Send final response to the coordinator
        ShardResponse::send_shard_response(shard_response, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
    }
}

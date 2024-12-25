#include "common.h"
#include "shard.h"
#include <iostream>
#include <map>
#include <string>

using namespace std;

mutex kv_store_mutex;

void shard(const int &rank) {
    cout << "Shard " << rank << " started...\n";
    map<int, string> kv_store;
    map<int, bool> prepared_keys;

    while (true) {
        auto [Type, key, value, recPhase] = ShardRequest::receive_shard_request(0, SHARD_REQUEST, MPI_COMM_WORLD);

        const auto phase = selectPhase(recPhase);
        const auto type = selectType(Type);

        ShardResponse shard_response{};

        if (type == READ) {
            lock_guard lock(kv_store_mutex);
            if (kv_store.contains(key)) {
                shard_response = {true, kv_store[key]};
            } else {
                shard_response = {false, "Key not found"};
            }

            // Send the response back to the coordinator
            ShardResponse::send_shard_response(shard_response, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
            continue;
        }

        if (phase == PREPARE) {
            lock_guard lock(kv_store_mutex);
            bool can_proceed = false;

            if (prepared_keys[key]) {
                ShardResponse::send_shard_response({false, "Already in PREPARE state"}, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
                continue;
            }
            switch (type) {
                case CREATE:
                    can_proceed = !kv_store.contains(key);
                break;
                case UPDATE:
                case DELETE:
                    can_proceed = kv_store.contains(key);
                break;
                default: ;
            }

            if (can_proceed) {
                prepared_keys[key] = true;
                ShardResponse::send_shard_response({true, "PREPARE success"}, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
            } else {
                ShardResponse::send_shard_response({false, "PREPARE failed"}, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
            }
            continue;
        }

        if (phase == COMMIT) {
            lock_guard lock(kv_store_mutex);
            if (!prepared_keys[key]) {
                ShardResponse::send_shard_response({false, "Not prepared"}, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
                continue;
            }

            bool success = true;
            string msg;

            switch (type) {
                case CREATE:
                case UPDATE:
                    kv_store[key] = value;
                    msg = value;
                    break;
                case DELETE:
                    kv_store.erase(key);
                    msg = "Deleted";
                    break;
                default:
                    success = false;
                    msg = "Invalid operation";
            }

            prepared_keys.erase(key);
            ShardResponse::send_shard_response({success, msg}, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
            continue;
        }

        if (phase == ROLLBACK) {
            prepared_keys.erase(key);
            ShardResponse::send_shard_response({false, "Rolled back"}, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
            continue;
        }

        ShardResponse::send_shard_response({false, "Invalid phase"}, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
    }
}

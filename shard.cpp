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
        auto [type, key, value] = ShardRequest::receive_shard_request(0, SHARD_REQUEST, MPI_COMM_WORLD);
        cout << "Shard " << rank << ": Received request for key " << key << "\n";

        ShardResponse shard_response{};

        switch (type) {
            case CREATE:
                if (!store.contains(key)) {
                    store[key] = value;
                    shard_response = {true, "Key created"};
                } else {
                    shard_response = {false, "Key already exists"};
                }
                break;

            case READ:
                if (store.contains(key)) {
                    shard_response = {true, store[key]};
                } else {
                    shard_response = {false, "Key not found"};
                }
                break;

            case UPDATE:
                if (store.contains(key)) {
                    store[key] = value;
                    shard_response = {true, "Key updated"};
                } else {
                    shard_response = {false, "Key not found"};
                }
                break;

            case DELETE:
                if (store.contains(key)) {
                    store.erase(key);
                    shard_response = {true, "Key deleted"};
                } else {
                    shard_response = {false, "Key not found"};
                }
                break;

            default:
                shard_response = {false, "Invalid request type"};
                break;
        }

        ShardResponse::send_shard_response(shard_response, 0, SHARD_RESPONSE, MPI_COMM_WORLD);
    }
}

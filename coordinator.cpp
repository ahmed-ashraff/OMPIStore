#include "common.h"
#include "coordinator.h"
#include <iostream>

using namespace std;

void coordinator(int world_size) {
    cout << "Coordinator started...\n";

    while (true) {
        auto [client_rank, type, key, value] = ClientRequest::receive_client_request(MPI_ANY_SOURCE, CLIENT_REQUEST, MPI_COMM_WORLD);

        string type_string;
        switch (type) {
            case 1:
                type_string= "CREATE";
                break;
            case 2:
                type_string = "READ";
                break;
            case 3:
                type_string = "UPDATE";
                break;
            case 4:
                type_string="DELETE";
        }

        cout << "Coordinator: Received request of type " << type_string << " for key " << key << "\n";

        const int shard_id = (key % (world_size - 1)) + 1; // Shards have ranks 1 to (world_size-1)
        cout << "Coordinator: Routing key " << key << " to shard " << shard_id << "\n";

        ShardRequest shard_request{type, key, value};
        ShardRequest::send_shard_request(shard_request, shard_id, SHARD_REQUEST, MPI_COMM_WORLD);

        auto response = ShardResponse::receive_shard_response(shard_id, SHARD_RESPONSE, MPI_COMM_WORLD);
        cout << "Coordinator: Forwarding response for key " << key << " to client " << client_rank << "\n";

        ClientRequest::send_client_response(response, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
    }
}


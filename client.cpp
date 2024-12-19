#include "common.h"
#include "client.h"
#include <iostream>
#include <string>

using namespace std;

void client(const int &rank) {
    cout << "Client started...\n";

    // CREATE operation
    const ClientRequest create_request{rank, CREATE, 1, "First Data In My Distributed DB"};
    ClientRequest::send_client_request(create_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    auto [success, value] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    cout << "Client: CREATE Response " << (success ? "Success" : "Failed") << ", " << value << "\n";

    // cout << '\n';
    //
    // // READ operation
    // const ClientRequest read_request{rank, READ, 1, ""};
    // ClientRequest::send_client_request(read_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);
    // const auto [success2, value2] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    // cout << "Client: READ Response " << (success2 ? "Success" : "Failed") << ", " << value2 << "\n";
    //
    // cout << '\n';
    //
    // // UPDATE operation
    // const ClientRequest update_request{rank, UPDATE, 123, "Updated Value"};
    // ClientRequest::send_client_request(update_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);
    //
    // auto [success3, value3] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    // cout << "Client: UPDATE Response " << (success3 ? "Success" : "Failed") << ", " << value3 << "\n";
    //
    // cout << '\n';
    //
    // // DELETE operation
    // const ClientRequest delete_request{rank, DELETE, 123, ""};
    // ClientRequest::send_client_request(delete_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);
    //
    // const auto [success5, value6] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    // cout << "Client: DELETE Response - " << (success5 ? "Success" : "Failed") << ", " << value6 << "\n";
}



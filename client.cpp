#include "common.h"
#include "client.h"
#include "logger.h"
#include <string>

using namespace std;

void client(const int &rank) {
    auto& logger = Logger::getInstance();

    logger.info("Client " + to_string(rank) + " started...", rank);

    // CREATE operation
    const ClientRequest create_request{rank, CREATE, 1, "Hello world"};
    ClientRequest::send_client_request(create_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    auto [success, value] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    logger.info("CREATE Response: " + string(success ? "Success" : "Failed") + ", Value: " + value, rank);

    // READ operation
    const ClientRequest read_request{rank, READ, 1, ""};
    ClientRequest::send_client_request(read_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    const auto [success2, value2] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    logger.info("READ Response: " + string(success2 ? "Success" : "Failed") + ", Value: " + value2, rank);

    // UPDATE operation
    const ClientRequest update_request{rank, UPDATE, 1, "Hello paxos"};
    ClientRequest::send_client_request(update_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    auto [success3, value3] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    logger.info("UPDATE Response: " + string(success3 ? "Success" : "Failed") + ", Value: " + value3, rank);

    // Second READ operation
    const ClientRequest read_request2{rank, READ, 1, ""};
    ClientRequest::send_client_request(read_request2, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    const auto [success4, value4] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    logger.info("Second READ Response: " + string(success4 ? "Success" : "Failed") + ", Value: " + value4, rank);

    // DELETE operation (commented out for now)
    // const ClientRequest delete_request{rank, DELETE, 2, ""};
    // ClientRequest::send_client_request(delete_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);
    //
    // const auto [success5, value5] = ShardResponse::receive_shard_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    // logger.info("DELETE Response: " + string(success5 ? "Success" : "Failed") + ", Value: " + value5, rank);

    logger.info("Exiting client...", rank);
}

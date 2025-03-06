#ifndef CLIENT_REQUEST_UTILS_H
#define CLIENT_REQUEST_UTILS_H

#include "types.h"
#include "mpi_manager.h"

void send_client_request(const ClientRequest &request, int dest, int tag, MPI_Comm comm);
ClientRequest receive_client_request(int source, int tag, MPI_Comm comm);

#endif //CLIENT_REQUEST_UTILS_H

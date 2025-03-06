#ifndef SEND_CLIENT_RESPONSE_H
#define SEND_CLIENT_RESPONSE_H

#include "types.h"
#include "mpi_manager.h"

void send_client_response(const NodeResponse& response, int dest, int tag, MPI_Comm comm);

#endif //SEND_CLIENT_RESPONSE_H

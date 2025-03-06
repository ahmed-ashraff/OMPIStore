#ifndef NODE_RESPONSE_UTILS_H
#define NODE_RESPONSE_UTILS_H

#include "types.h"
#include "mpi_manager.h"

void send_node_response(const NodeResponse &response, int dest, int tag, MPI_Comm comm);
NodeResponse receive_node_response(int source, int tag, MPI_Comm comm);

#endif //NODE_RESPONSE_UTILS_H

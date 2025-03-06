#ifndef NODE_REQUEST_UTILS_H
#define NODE_REQUEST_UTILS_H

#include "types.h"
#include "mpi_manager.h"

void send_node_request(const NodeRequest &request, int dest, int tag, MPI_Comm comm);
NodeRequest receive_node_request(int source, int tag, MPI_Comm comm);

#endif //NODE_REQUEST_UTILS_H

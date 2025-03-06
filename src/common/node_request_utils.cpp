#include "../../include/common/node_request_utils.h"
#include "../../include/common/mpi_manager.h"
#include "../../include/common/types.h"

void send_node_request(const NodeRequest &request, int dest, int tag, MPI_Comm comm) {
    mpi_manager::send_enum(static_cast<int>(request.type), dest, tag, comm);
    mpi_manager::send_int(request.key, dest, tag + 1, comm);
    mpi_manager::send_string(request.value, dest, tag + 2, comm);
    mpi_manager::send_enum(static_cast<int>(request.state), dest, tag + 3, comm);
}

NodeRequest receive_node_request(int source, int tag, MPI_Comm comm) {
    NodeRequest request;
    request.type = mpi_manager::receive_request_type(source, tag, comm);
    request.key = mpi_manager::receive_int(source, tag + 1, comm);
    request.value = mpi_manager::receive_string(source, tag + 2, comm);
    request.state = mpi_manager::receive_phase_type(source, tag + 3, comm);
    return request;
}
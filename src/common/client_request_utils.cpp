#include "../../include/common/client_request_utils.h"
#include "../../include/common/mpi_manager.h"
#include "../../include/common/types.h"

void send_client_request(const ClientRequest &request, int dest, int tag, MPI_Comm comm) {
    mpi_manager::send_int(request.client_rank, dest, tag, comm);
    mpi_manager::send_enum(static_cast<int>(request.type), dest, tag + 1, comm);
    mpi_manager::send_int(request.key, dest, tag + 2, comm);
    mpi_manager::send_string(request.value, dest, tag + 3, comm);
}

ClientRequest receive_client_request(int source, int tag, MPI_Comm comm) {
    ClientRequest request;
    request.client_rank = mpi_manager::receive_int(source, tag, comm);
    request.type = mpi_manager::receive_request_type(source, tag + 1, comm);
    request.key = mpi_manager::receive_int(source, tag + 2, comm);
    request.value = mpi_manager::receive_string(source, tag + 3, comm);
    return request;
}
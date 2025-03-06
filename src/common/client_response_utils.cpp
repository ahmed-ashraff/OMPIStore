#include "../../include/common/client_response_utils.h"
#include "../../include/common/mpi_manager.h"
#include "../../include/common/types.h"

void send_client_response(const NodeResponse& response, int dest, int tag, MPI_Comm comm) {
    mpi_manager::send_bool(response.success, dest, tag, comm);
    mpi_manager::send_string(response.value, dest, tag + 1, comm);
}
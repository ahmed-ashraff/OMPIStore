#include <bits/stdc++.h>
#include "mpi.h"
#include "coordinator.h"
#include "node.h"
#include "client.h"

using namespace std;


int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == 0) {
        coordinator(size-2);
    } else if (rank == 1 || rank == 2) {
        node(rank);
    } else {
        client(rank);
    }

    MPI_Finalize();
    return 0;
}
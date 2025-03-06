#ifndef MPI_MANAGER_H
#define MPI_MANAGER_H

#include <mpi.h>
#include <bits/stdc++.h>

enum class RequestType : int;
enum class TwoPC : int;

using namespace std;


namespace mpi_manager {
    void send_string(const string &str, int dest, int tag, MPI_Comm comm);

    string receive_string(int source, int tag, MPI_Comm comm);

    void send_bool(const bool &value, int dest, int tag, MPI_Comm comm);

    bool receive_bool(int source, int tag, MPI_Comm comm);

    void send_enum(const int &value, int dest, int tag, MPI_Comm comm);

    void send_int(const int &value, int dest, int tag, MPI_Comm comm);

    int receive_int(int source, int tag, MPI_Comm comm);

    RequestType receive_request_type(int source, int tag, MPI_Comm comm);

    TwoPC receive_phase_type(int source, int tag, MPI_Comm comm);
}

#endif //MPI_MANAGER_H

#include "../../include/common/mpi_manager.h"
#include <bits/stdc++.h>
#include <omp.h>

using namespace std;

namespace mpi_manager {
    void send_string(const string &str, const int dest, const int tag, MPI_Comm comm) {
        const int str_size = static_cast<int>(str.size());
        // Use well-separated tags to avoid conflicts
        const int size_tag = tag * 2;
        const int data_tag = tag * 2 + 1;

        MPI_Send(&str_size, 1, MPI_INT, dest, size_tag, comm);
        if (str_size > 0) {
            MPI_Send(str.c_str(), str_size, MPI_CHAR, dest, data_tag, comm);
        }
    }

    string receive_string(const int source, const int tag, MPI_Comm comm) {
        int str_size;
        const int size_tag = tag * 2;
        const int data_tag = tag * 2 + 1;

        MPI_Recv(&str_size, 1, MPI_INT, source, size_tag, comm, MPI_STATUS_IGNORE);

        if (str_size > 0) {
            vector<char> buffer(str_size);
            MPI_Recv(buffer.data(), str_size, MPI_CHAR, source, data_tag, comm, MPI_STATUS_IGNORE);
            return string(buffer.begin(), buffer.end());
        }
        return "";
    }

    void send_int(const int &value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_INT, dest, tag, comm);
    }

    void send_bool(const bool &value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_CXX_BOOL, dest, tag, comm);
    }

    void send_enum(const int &value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_INT, dest, tag, comm);
    }

    int receive_int(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return value;
    }

    bool receive_bool(const int source, const int tag, MPI_Comm comm) {
        bool value = false;
        MPI_Recv(&value, 1, MPI_CXX_BOOL, source, tag, comm, MPI_STATUS_IGNORE);
        return value;
    }

    RequestType receive_request_type(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return static_cast<RequestType>(value);
    }

    TwoPC receive_phase_type(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return static_cast<TwoPC>(value);
    }
}

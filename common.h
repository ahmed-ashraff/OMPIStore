#ifndef COMMON_H
#define COMMON_H

#include <mpi.h>
#include <bits/stdc++.h>

using namespace std;

enum RequestType {
    CREATE = 1,
    READ = 2,
    UPDATE = 3,
    DELETE = 4
};

enum MessageType {
    CLIENT_REQUEST,
    CLIENT_RESPONSE,
    SHARD_REQUEST,
    SHARD_RESPONSE
};

namespace mpi_utils {
    inline void send_string(const string& str, const int dest, const int tag, MPI_Comm comm) {
        const int str_size = static_cast<int>(str.size());
        MPI_Send(&str_size, 1, MPI_INT, dest, tag, comm);
        MPI_Send(str.c_str(), str_size, MPI_CHAR, dest, tag + 1, comm);
    }

    inline string receive_string(const int source, const int tag, MPI_Comm comm) {
        int str_size;
        MPI_Recv(&str_size, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);

        vector<char> buffer(str_size);
        MPI_Recv(buffer.data(), str_size, MPI_CHAR, source, tag + 1, comm, MPI_STATUS_IGNORE);

        return string(buffer.begin(), buffer.end());
    }

    inline void send_int(const int& value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_INT, dest, tag, comm);
    }

    inline void send_bool(const bool& value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_CXX_BOOL, dest, tag, comm);
    }

    inline void send_enum(const int& value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_INT, dest, tag, comm);
    }

    inline int receive_int(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return value;
    }

    inline bool receive_bool(const int source, const int tag, MPI_Comm comm) {
        bool value;
        MPI_Recv(&value, 1, MPI_CXX_BOOL, source, tag, comm, MPI_STATUS_IGNORE);
        return value;
    }

    inline RequestType receive_request_type(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return static_cast<RequestType>(value);
    }
}

struct ShardResponse {
    bool success{};
    string value;

    static void send_shard_response(const ShardResponse& response, const int dest, const int tag, MPI_Comm comm) {
        mpi_utils::send_bool(response.success, dest, tag, comm);
        mpi_utils::send_string(response.value, dest, tag + 1, comm);
    }

    static ShardResponse receive_shard_response(const int source, const int tag, MPI_Comm comm) {
        ShardResponse response;
        response.success = mpi_utils::receive_bool(source, tag, comm);
        response.value = mpi_utils::receive_string(source, tag + 1, comm);
        return response;
    }
};

struct ClientRequest {
    int client_rank{};
    RequestType type;
    int key{};
    string value;

    static void send_client_request(const ClientRequest& request, const int dest, const int tag, MPI_Comm comm) {
        mpi_utils::send_int(request.client_rank, dest, tag, comm);
        mpi_utils::send_enum(request.type, dest, tag + 1, comm);
        mpi_utils::send_int(request.key, dest, tag + 2, comm);
        mpi_utils::send_string(request.value, dest, tag + 3, comm);
    }

    static void send_client_response(const ShardResponse& response, const int dest, const int tag, MPI_Comm comm) {
        mpi_utils::send_bool(response.success, dest, tag, comm);
        mpi_utils::send_string(response.value, dest, tag + 1, comm);
    }

    static ClientRequest receive_client_request(const int source, const int tag, MPI_Comm comm) {
        ClientRequest request;
        request.client_rank = mpi_utils::receive_int(source, tag, comm);
        request.type = mpi_utils::receive_request_type(source, tag + 1, comm);
        request.key = mpi_utils::receive_int(source, tag + 2, comm);
        request.value = mpi_utils::receive_string(source, tag + 3, comm);
        return request;
    }
};

struct ShardRequest {
    RequestType type;
    int key{};
    string value;

    static void send_shard_request(const ShardRequest& request, const int dest, const int tag, MPI_Comm comm) {
        mpi_utils::send_enum(request.type, dest, tag, comm);
        mpi_utils::send_int(request.key, dest, tag + 1, comm);
        mpi_utils::send_string(request.value, dest, tag + 2, comm);
    }

    static ShardRequest receive_shard_request(const int source, const int tag, MPI_Comm comm) {
        ShardRequest request;
        request.type = mpi_utils::receive_request_type(source, tag, comm);
        request.key = mpi_utils::receive_int(source, tag + 1, comm);
        request.value = mpi_utils::receive_string(source, tag + 2, comm);
        return request;
    }
};

#endif //COMMON_H
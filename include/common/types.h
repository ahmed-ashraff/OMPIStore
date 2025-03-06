#ifndef TYPES_H
#define TYPES_H

#include <bits/stdc++.h>

using namespace std;

enum MessageType {
    CLIENT_REQUEST,
    CLIENT_RESPONSE,
    NODE_REQUEST,
    NODE_RESPONSE
};

enum class RequestType : int {
    CREATE = 1,
    READ = 2,
    UPDATE = 3,
    DELETE = 4
};

enum class TwoPC : int {
    PREPARE = 1,
    COMMIT = 2,
    ROLLBACK = 3
};

struct NodeResponse {
    bool success{};
    string value;
};

struct ClientRequest {
    int client_rank{};
    RequestType type{};
    int key{};
    string value;
};

struct NodeRequest {
    RequestType type{};
    int key{};
    string value;
    TwoPC state{};
};

#endif //TYPES_H

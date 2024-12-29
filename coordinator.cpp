#include "common.h"
#include "coordinator.h"
#include "logger.h"
#include "mpi.h"
#include <chrono>
#include <fstream>

using namespace std;

// Global lock table
map<int, bool> key_locks;
mutex lock_mutex;

bool is_key_locked(int key) {
    lock_guard guard(lock_mutex);
    return key_locks[key];
}

void lock_key(int key) {
    lock_guard guard(lock_mutex);
    key_locks[key] = true;
}

void unlock_key(int key) {
    lock_guard guard(lock_mutex);
    key_locks[key] = false;
}

inline void log_performance(const string& operation, double duration, size_t memory_usage = 0) {
    ofstream log_file("performance_log.txt", ios_base::app);
    if (!log_file.is_open()) {
        cerr << "Error opening log file!" << endl;
        return;
    }
    log_file << "Operation: " << operation << ", Duration: " << duration << " seconds";
    if (memory_usage > 0) {
        log_file << ", Memory Usage: " << memory_usage << " KB";
    }
    log_file << "\n";
    log_file.close();
}

inline size_t get_memory_usage() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return pmc.PrivateUsage / 1024; // Convert bytes to KB
#else
    ifstream status_file("/proc/self/status");
    string line;
    size_t memory_usage = 0;

    while (getline(status_file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            istringstream iss(line);
            string key;
            iss >> key >> memory_usage; // Read the memory usage in KB
            break;
        }
    }

    return memory_usage;
#endif
}

void coordinator(const int &nodes) {
    auto& logger = Logger::getInstance();
    logger.info("Coordinator started...\n", 0);

    while (true) {
        auto start_time = chrono::high_resolution_clock::now();

        auto [client_rank, Type, key, value] = ClientRequest::receive_client_request(MPI_ANY_SOURCE, CLIENT_REQUEST, MPI_COMM_WORLD);
        const auto type = selectType(Type);

        if (type == READ) {
            vector<NodeResponse> responses;

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                NodeRequest read_request{READ, key, ""};
                NodeRequest::send_node_request(read_request, node_id, NODE_REQUEST, MPI_COMM_WORLD);

                auto response = NodeResponse::receive_node_response(node_id, NODE_RESPONSE, MPI_COMM_WORLD);
                responses.push_back(response);
            }

            string latest_value;
            bool found = false;

            for (auto &[success, value] : responses) {
                if (!found && success) {
                    latest_value = value;
                    found = true;
                }
            }

            if (found) {
                ClientRequest::send_client_response({true, latest_value}, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
            } else {
                ClientRequest::send_client_response({false, "Key not found"}, client_rank, CLIENT_RESPONSE, MPI_COMM_WORLD);
            }
        } else {
            if (is_key_locked(key)) {
                logger.warning("Key " + to_string(key) + " is already locked, rejecting request", 0);
                ClientRequest::send_client_response(
                    {false, "Key is locked by another transaction"},
                    client_rank,
                    CLIENT_RESPONSE,
                    MPI_COMM_WORLD
                );
                continue;
            }

            lock_key(key);
            logger.debug("Locked key: " + to_string(key), 0);

            bool prepare_success = true;

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                NodeRequest node_request{type, key, value, PREPARE};
                NodeRequest::send_node_request(node_request, node_id, NODE_REQUEST, MPI_COMM_WORLD);
            }

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                const auto [success, str] = NodeResponse::receive_node_response(node_id, NODE_RESPONSE, MPI_COMM_WORLD);
                logger.info("Received from node " + to_string(node_id) + ": " + str, 0);
                prepare_success = prepare_success && success;
            }

            logger.info("Prepare phase success = " + string(prepare_success ? "true" : "false"), 0);

            const TwoPC next_phase = prepare_success ? COMMIT : ROLLBACK;
            logger.info("Starting " + string(next_phase == COMMIT ? "COMMIT" : "ROLLBACK") + " phase", 0);

            for (int node_id = 1; node_id <= nodes; ++node_id) {
                NodeRequest node_request{type, key, value, next_phase};
                NodeRequest::send_node_request(node_request, node_id, NODE_REQUEST, MPI_COMM_WORLD);
                NodeResponse::receive_node_response(node_id, NODE_RESPONSE, MPI_COMM_WORLD);
            }

            unlock_key(key);
            logger.info("Starting " + string(next_phase == COMMIT ? "COMMIT" : "ROLLBACK") + " phase", 0);

            ClientRequest::send_client_response(
                {prepare_success, prepare_success ? value : "Operation failed"},
                client_rank,
                CLIENT_RESPONSE,
                MPI_COMM_WORLD
            );
        }

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end_time - start_time;
        size_t memory_usage = get_memory_usage();
        log_performance("Coordinator Operation", duration.count(), memory_usage);
    }
}
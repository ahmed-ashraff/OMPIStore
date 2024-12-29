#include "common.h"
#include "node.h"
#include "logger.h"
#include <map>
#include <string>
#include <chrono>
#include <fstream>

using namespace std;

mutex kv_store_mutex;

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

void node(const int &rank) {
    auto& logger = Logger::getInstance();
    logger.info("Node " + to_string(rank) + " started...", rank);

    map<int, string> kv_store;
    map<int, bool> prepared_keys;

    while (true) {
        auto start_time = chrono::high_resolution_clock::now();

        auto [Type, key, value, recPhase] = NodeRequest::receive_node_request(0, NODE_REQUEST, MPI_COMM_WORLD);

        const auto phase = selectPhase(recPhase);
        const auto type = selectType(Type);

        NodeResponse node_response{};

        if (type == READ) {
            lock_guard lock(kv_store_mutex);
            if (kv_store.contains(key)) {
                node_response = {true, kv_store[key]};
                logger.debug("READ success for key: " + to_string(key), rank);
            } else {
                node_response = {false, "Key not found"};
                logger.debug("READ failed - key not found: " + to_string(key), rank);
            }

            NodeResponse::send_node_response(node_response, 0, NODE_RESPONSE, MPI_COMM_WORLD);
        } else if (phase == PREPARE) {
            lock_guard lock(kv_store_mutex);
            bool can_proceed = false;

            if (prepared_keys[key]) {
                logger.warning("Key " + to_string(key) + " already in PREPARE state", rank);
                NodeResponse::send_node_response({false, "Already in PREPARE state"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            } else {
                switch (type) {
                    case CREATE:
                        can_proceed = !kv_store.contains(key);
                        break;
                    case UPDATE:
                    case DELETE:
                        can_proceed = kv_store.contains(key);
                        break;
                    default:
                        break;
                }

                if (can_proceed) {
                    prepared_keys[key] = true;
                    logger.info("PREPARE success for key: " + to_string(key), rank);
                    NodeResponse::send_node_response({true, "PREPARE success"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
                } else {
                    logger.warning("PREPARE failed for key: " + to_string(key), rank);
                    NodeResponse::send_node_response({false, "PREPARE failed"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
                }
            }
        } else if (phase == COMMIT) {
            lock_guard lock(kv_store_mutex);
            if (!prepared_keys[key]) {
                logger.error("COMMIT failed - key not prepared: " + to_string(key), rank);
                NodeResponse::send_node_response({false, "Not prepared"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            } else {
                bool success = true;
                string msg;

                switch (type) {
                    case CREATE:
                    case UPDATE:
                        kv_store[key] = value;
                        msg = value;
                        logger.info("COMMIT success - " + string(type == CREATE ? "Created" : "Updated") +
                                   " key: " + to_string(key) + " with value: " + value, rank);
                        break;
                    case DELETE:
                        kv_store.erase(key);
                        msg = "Deleted";
                        logger.info("COMMIT success - Deleted key: " + to_string(key), rank);
                        break;
                    default:
                        success = false;
                        msg = "Invalid operation";
                        logger.error("COMMIT failed - Invalid operation for key: " + to_string(key), rank);
                }

                prepared_keys.erase(key);
                NodeResponse::send_node_response({success, msg}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
            }
        } else if (phase == ROLLBACK) {
            prepared_keys.erase(key);
            logger.info("ROLLBACK executed for key: " + to_string(key), rank);
            NodeResponse::send_node_response({false, "Rolled back"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
        } else {
            logger.error("Invalid phase received for key: " + to_string(key), rank);
            NodeResponse::send_node_response({false, "Invalid phase"}, 0, NODE_RESPONSE, MPI_COMM_WORLD);
        }

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end_time - start_time;
        size_t memory_usage = get_memory_usage();
        log_performance("Node "+ to_string(rank)+" Operation ", duration.count(), memory_usage);
    }
    }

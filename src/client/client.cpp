#include "../../include/client/client.h"
#include "../../include/common/logger.h"
#include "../../include/common/types.h"
#include <string>
#include <chrono>
#include <fstream>

#include "../../include/common/client_request_utils.h"
#include "../../include/common/node_response_utils.h"

using namespace std;

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

void client(const int &rank) {
    auto& logger = Logger::getInstance();
    logger.info("Client " + to_string(rank) + " started...", rank);

    // CREATE operation
    auto start_time = chrono::high_resolution_clock::now();
    const ClientRequest create_request{rank, RequestType::CREATE, 1, "Hello world"};
    send_client_request(create_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    auto [success, value] = receive_node_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end_time - start_time;
    size_t memory_usage = get_memory_usage();
    log_performance("Client CREATE", duration.count(), memory_usage);
    logger.info("CREATE Response: " + string(success ? "Success" : "Failed") + ", Value: " + value, rank);

    // READ operation
    start_time = chrono::high_resolution_clock::now();
    const ClientRequest read_request{rank, RequestType::READ, 1, ""};
    send_client_request(read_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    const auto [success2, value2] = receive_node_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    end_time = chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    memory_usage = get_memory_usage();
    log_performance("Client READ", duration.count(), memory_usage);
    logger.info("READ Response: " + string(success2 ? "Success" : "Failed") + ", Value: " + value2, rank);

    // UPDATE operation
    start_time = chrono::high_resolution_clock::now();
    const ClientRequest update_request{rank, RequestType::UPDATE, 1, "Hello paxos"};
    send_client_request(update_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    auto [success3, value3] = receive_node_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    end_time = chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    memory_usage = get_memory_usage();
    log_performance("Client UPDATE", duration.count(), memory_usage);
    logger.info("UPDATE Response: " + string(success3 ? "Success" : "Failed") + ", Value: " + value3, rank);

    // Second READ operation
    start_time = chrono::high_resolution_clock::now();
    const ClientRequest read_request2{rank, RequestType::READ, 1, ""};
    send_client_request(read_request2, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    const auto [success4, value4] = receive_node_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    end_time = chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    memory_usage = get_memory_usage();
    log_performance("Client READ 2", duration.count(), memory_usage);
    logger.info("Second READ Response: " + string(success4 ? "Success" : "Failed") + ", Value: " + value4, rank);

    // DELETE operation
    start_time = chrono::high_resolution_clock::now();
    const ClientRequest delete_request{rank, RequestType::DELETE, 2, ""};
    send_client_request(delete_request, 0, CLIENT_REQUEST, MPI_COMM_WORLD);

    const auto [success5, value5] = receive_node_response(0, CLIENT_RESPONSE, MPI_COMM_WORLD);
    end_time = chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    memory_usage = get_memory_usage();
    log_performance("Client DELETE", duration.count(), memory_usage);
    logger.info("DELETE Response: " + string(success5 ? "Success" : "Failed") + ", Value: " + value5, rank);

    logger.info("Exiting client...", rank);
}
#include <bits/stdc++.h>
#include "mpi.h"
#include "../include/coordinator/coordinator.h"
#include "../include/node/node.h"
#include "../include/client/client.h"
#include <chrono>
#include <fstream>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <fstream>
#include <sstream>
#endif
using namespace std;

inline void log_performance(const string &operation, double duration, size_t memory_usage = 0) {
    ofstream log_file("performance_log.txt", ios_base::app);
    if (!log_file.is_open()) {
        cerr << "Error opening log file!" << endl;
        return;
    }
    log_file << "Operation: " << operation << ", Duration: " << duration << " seconds";

    log_file << ", Memory Usage: " << memory_usage << " KB";

    log_file << "\n";
    log_file.close();
}

inline size_t get_memory_usage() {
#ifdef _WIN32
    // Windows-specific memory usage measurement
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return pmc.PrivateUsage / 1024; // Convert bytes to KB
#else
    // Linux-specific memory usage measurement
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

int main(int argc, char **argv) {
    const auto start_time = chrono::high_resolution_clock::now();

    int rank, size;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        coordinator(size - 2);
    } else if (rank == 1 || rank == 2) {
        node(rank);
    } else {
        client(rank);
    }
    const auto end_time = chrono::high_resolution_clock::now();
    const chrono::duration<double> duration = end_time - start_time;
    const size_t memory_usage = get_memory_usage();
    log_performance("Total Execution", duration.count(), memory_usage);

    MPI_Finalize();

    return 0;
}

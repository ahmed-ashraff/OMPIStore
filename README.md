# Distributed Key-Value Store

## Overview
This project is a **Distributed Key-Value Store** implemented in C++. It allows multiple nodes to store and retrieve key-value pairs in a distributed manner. A **coordinator** manages the key-value storage across multiple **nodes**, while a **client** interacts with the system to perform operations. The system uses **message passing** for communication between components.

## Features
- **Distributed storage** of key-value pairs
- **Fault tolerance** via multiple nodes
- **Centralized coordination** through a coordinator
- **Client API** to interact with the system
- **Logging** for debugging and monitoring

## Directory Structure
```
├── README.md             # Project documentation
├── CMakeLists.txt        # CMake build configuration
├── client.cpp            # Client implementation
├── client.h              # Client header file
├── common.h              # Shared definitions
├── coordinator.cpp       # Coordinator implementation
├── coordinator.h         # Coordinator header file
├── logger.cpp            # Logging implementation
├── logger.h              # Logger header file
├── main.cpp              # Entry point of the program
├── makefile              # Alternative build configuration
├── node.cpp              # Node implementation
├── node.h                # Node header file
└── cmake-build-debug/    # Build directory (generated files)
```

## Components
### 1. **Client**
The client interacts with the distributed system by sending requests to store, retrieve, and delete key-value pairs.

### 2. **Coordinator**
The coordinator is responsible for managing nodes, distributing keys, and ensuring data consistency.

### 3. **Nodes**
Each node stores a portion of the key-value data and communicates with the coordinator.

### 4. **Logger**
The logger module records system events for debugging and monitoring purposes.

## Build Instructions
### Using CMake
```sh
mkdir build && cd build
cmake ..
make
```

### Using Makefile
```sh
make
```

## Usage
1. Start the **coordinator**
```sh
./coordinator
```
2. Start **nodes**
```sh
./node
```
3. Run the **client** to interact with the system
```sh
./client
```

## Contributing
Just submit a pull request whenver you want to contribute!


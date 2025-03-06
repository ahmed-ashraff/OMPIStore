cmake_minimum_required(VERSION 3.28)
project(openmp_mpi)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
# Find OpenMP
find_package(OpenMP REQUIRED)

# Find MPI
find_package(MPI REQUIRED)

add_executable(openmp_mpi src/main.cpp
        src/node/node.cpp
        src/coordinator/coordinator.cpp
        src/client/client.cpp
        include/node/node.h
        include/coordinator/coordinator.h
        include/client/client.h
        include/common/logger.h
        src/common/logger.cpp
        include/common/types.h
        include/common/mpi_manager.h
        src/common/mpi_manager.cpp
        include/common/client_request_utils.h
        include/common/node_response_utils.h
        include/common/node_request_utils.h
        src/common/client_request_utils.cpp
        src/common/node_response_utils.cpp
        src/common/node_request_utils.cpp
        include/common/client_response_utils.h
        src/common/client_response_utils.cpp
        src/main.cpp
)

# Link OpenMP
if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(include)

# Link MPI
if(MPI_CXX_FOUND)
    target_include_directories(openmp_mpi PRIVATE ${MPI_INCLUDE_PATH})
    target_link_libraries(openmp_mpi MPI::MPI_CXX)
endif()


cmake_minimum_required(VERSION 3.28)
project(openmp_mpi)

set(CMAKE_CXX_STANDARD 26)

# Find OpenMP
#find_package(OpenMP REQUIRED)

# Find MPI
find_package(MPI REQUIRED)

add_executable(openmp_mpi main.cpp
        common.h
        node.cpp
        coordinator.cpp
        client.cpp
        node.h
        coordinator.h
        client.h
        logger.h
        logger.cpp)

# Link OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(openmp_mpi OpenMP::OpenMP_CXX)
endif()

# Link MPI
if(MPI_CXX_FOUND)
    target_include_directories(openmp_mpi PRIVATE ${MPI_INCLUDE_PATH})
    target_link_libraries(openmp_mpi MPI::MPI_CXX)
endif()


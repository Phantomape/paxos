# Use CMake Tools extension(optional)
include(CMakeToolsHelpers OPTIONAL)

cmake_minimum_required(VERSION 2.8)

project(Paxos)

# The version number. More work need to be done
set (Paxos_VERSION_MAJOR 1)
set (Paxos_VERSION_MINOR 0)

set (CMAKE_CXX_STANDARD 11)

# Check compilers
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        message(FATAL_ERROR "GCC is too old, please install a newer version")
    endif()
else()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with GCC.")
endif()

find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
else()
    message(STATUS "LEVELDB:" ${LEVELDB_INCLUDE_PATH})
endif()

find_path(GFLAGS_INCLUDE_PATH NAMES gflags/gflags.h)
find_library(GFLAGS_LIB NAMES gflags)
if ((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIB))
    message(FATAL_ERROR "Fail to find gflags")
else()
    message(STATUS "GFLAGS:" ${GFLAGS_INCLUDE_PATH})
endif()

find_path(GRPC_INCLUDE_PATH NAMES grpc++/grpc++.h)
find_library(GRPC_LIB NAMES grpc)
if ((NOT GRPC_INCLUDE_PATH) OR (NOT GRPC_LIB))
    message(FATAL_ERROR "Fail to find gRpc")
else()
    message(STATUS "GRPC:" ${GRPC_INCLUDE_PATH})
endif()

aux_source_directory(. DIR_ROOT_SRCS)

# Include header files
include_directories(include)
include_directories(proto)
include_directories(third_party/leveldb/build/include)

add_subdirectory(src/comm)
add_subdirectory(src/conf)
add_subdirectory(src/core)
add_subdirectory(src/utils)
add_subdirectory(proto)

add_executable(Paxos test.cc ${DIR_ROOT_SRCS})

# Link a target to given libraries, e.g. Paxos is linked to CoreLib
target_link_libraries(Paxos core comm util conf foo protobuf)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin/")
install(TARGETS Paxos RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

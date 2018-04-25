# Use CMake Tools extension(optional)
include(CMakeToolsHelpers OPTIONAL)

cmake_minimum_required(VERSION 2.8)

project(Paxos)

# The version number.
set (Paxos_VERSION_MAJOR 1)
set (Paxos_VERSION_MINOR 0)

set (CMAKE_CXX_STANDARD 11)

aux_source_directory(. DIR_ROOT_SRCS)

# Include header files
include_directories(src/core)

add_subdirectory(src/core)
add_subdirectory(proto)

add_executable(Paxos test.cc ${DIR_ROOT_SRCS})

# Link a target to given libraries, e.g. Paxos is linked to CoreLib
target_link_libraries(Paxos CoreLib)

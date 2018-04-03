# Use CMake Tools extension(optional)
include(CMakeToolsHelpers OPTIONAL)

cmake_minimum_required(VERSION 2.8)

project(Paxos)

# The version number.
set (Paxos_VERSION_MAJOR 1)
set (Paxos_VERSION_MINOR 0)

aux_source_directory(. DIR_ROOT_SRCS)

# Include header files
include_directories(src/Core)

add_subdirectory(src/Core)

add_executable(Paxos test.cc ${DIR_ROOT_SRCS})

target_link_libraries(Paxos CoreLib)
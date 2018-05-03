include(CMakeToolsHelpers OPTIONAL)

cmake_minimum_required(VERSION 2.8)

aux_source_directory(. DIR_LIB_SRCS)

add_library(Comm STATIC ${DIR_LIB_SRCS})

target_link_libraries(Comm foo protobuf)

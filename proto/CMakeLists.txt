include(CMakeToolsHelpers OPTIONAL)

find_package(Protobuf REQUIRED)

# Generate *.pb.h and *.pb.cc and save it to PROTO_SRCS and PROTO_HDRS
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS paxos_msg.proto rpc.proto)

# Add a library to the project
add_library(foo ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(foo ${PROTOBUF_LIBRARIES} pthread)

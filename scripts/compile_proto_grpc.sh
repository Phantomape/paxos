cd && cd ~/Projects/paxos/scripts
protoc -I=../proto --grpc_out=../proto --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin rpc.proto

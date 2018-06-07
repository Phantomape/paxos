cd && cd ~/Projects/paxos/scripts
protoc -I=../proto --cpp_out=../proto paxos_msg.proto
protoc -I=../proto --cpp_out=../proto master_sm.proto
protoc -I=../proto --cpp_out=../proto rpc.proto
cd && cd ~/Projects/paxos/scripts
protoc -I=../proto --cpp_out=../src/core paxos_msg.proto
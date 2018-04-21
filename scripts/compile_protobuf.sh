cd && cd ~/Projects/paxos/scripts
protoc -I=../src/proto --cpp_out=../src/core test_paxos_msg.proto
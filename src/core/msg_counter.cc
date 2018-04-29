#include <iostream>
#include <string>
#include "msg_counter.h"

namespace paxos {
    
MsgCounter::MsgCounter() {
    std::cout << "MsgCounter::MsgCounter()" << std::endl;
}

MsgCounter::~MsgCounter() {
    std::cout << "MsgCounter::~MsgCounter()" << std::endl;
}

void MsgCounter::AddAcceptedMsg(const uint64_t node_id) {
    std::cout << "MsgCounter::AddAcceptedMsg()" << std::endl;
}

void MsgCounter::AddReceivedMsg(const uint64_t node_id) {
    std::cout << "MsgCounter::AddReceivedMsg()" << std::endl;
}

void MsgCounter::AddRejectedMsg(const uint64_t node_id) {
    std::cout << "MsgCounter::AddRejectedMsg()" << std::endl;
}

void MsgCounter::Init() {
    std::cout << "MsgCounter::Init()" << std::endl;
    ac_msg_node_ids.clear();
    recv_msg_node_ids.clear();
    rej_msg_node_ids.clear();
}

bool MsgCounter::IsPassed() {
    std::cout << "MsgCounter::IsPassed()" << std::endl;
}

bool MsgCounter::IsRejected() {
    std::cout << "MsgCounter::IsRejected()" << std::endl;
}

}


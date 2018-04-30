#include <iostream>
#include <string>
#include "msg_counter.h"

namespace paxos {
    
MsgCounter::MsgCounter() {
    std::cout << "MsgCounter::MsgCounter()" << std::endl;
    // Read config
    Init();
}

MsgCounter::~MsgCounter() {
    std::cout << "MsgCounter::~MsgCounter()" << std::endl;
}

void MsgCounter::AddAcceptedMsg(const uint64_t node_id) {
    std::cout << "MsgCounter::AddAcceptedMsg()" << std::endl;
    if (ac_msg_node_ids.find(node_id) == ac_msg_node_ids.end()) {
        ac_msg_node_ids.insert(node_id);
    }
}

void MsgCounter::AddReceivedMsg(const uint64_t node_id) {
    std::cout << "MsgCounter::AddReceivedMsg()" << std::endl;
    if (recv_msg_node_ids.find(node_id) == recv_msg_node_ids.end()) {
        recv_msg_node_ids.insert(node_id);
    }
}

void MsgCounter::AddRejectedMsg(const uint64_t node_id) {
    std::cout << "MsgCounter::AddRejectedMsg()" << std::endl;
    if (rej_msg_node_ids.find(node_id) == rej_msg_node_ids.end()) {
        rej_msg_node_ids.insert(node_id);
    }
}

void MsgCounter::Init() {
    std::cout << "MsgCounter::Init()" << std::endl;
    ac_msg_node_ids.clear();
    recv_msg_node_ids.clear();
    rej_msg_node_ids.clear();
}

bool MsgCounter::IsAllReceived() {
    std::cout << "MsgCounter::IsAllReceived()" << std::endl;
}

bool MsgCounter::IsPassed() {
    std::cout << "MsgCounter::IsPassed()" << std::endl;
}

bool MsgCounter::IsRejected() {
    std::cout << "MsgCounter::IsRejected()" << std::endl;
}

}


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

void MsgCounter::AddAcceptedMsg() {
    std::cout << "MsgCounter::AddAcceptedMsg()" << std::endl;
}

void MsgCounter::AddReceivedMsg() {
    std::cout << "MsgCounter::AddReceivedMsg()" << std::endl;
}

void MsgCounter::AddRejectedMsg() {
    std::cout << "MsgCounter::AddRejectedMsg()" << std::endl;
}

void MsgCounter::IsPassed() {
    std::cout << "MsgCounter::IsPassed()" << std::endl;
}

void MsgCounter::IsRejected() {
    std::cout << "MsgCounter::IsRejected()" << std::endl;
}


}


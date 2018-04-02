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

}

void MsgCounter::AddRejectedMsg() {
    
}

}


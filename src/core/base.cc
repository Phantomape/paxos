#include "base.h"
#include <iostream>
#include <string>

namespace paxos {
    
Base::Base() {
    std::cout << "Base::Base()" << std::endl;
}

Base::~Base() {
    std::cout << "Base::~Base()" << std::endl;
}

int Base::BroadcastMessage(const PaxosMsg &paxos_msg) {
    std::cout << "Base::BroadcastMessage()" << std::endl;
}

uint64_t Base::GetInstanceId() {
    std::cout << "Base::GetInstanceId()" << std::endl;
    return instance_id_;
}

}


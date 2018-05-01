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
    if (is_test_mode_) {
        return 0;
    }

    // Do sth.
}

uint64_t Base::GetInstanceId() {
    std::cout << "Base::GetInstanceId()" << std::endl;
    return instance_id_;
}

int Base::UnpackBaseMsg(const std::string& str, Header& header, size_t& body_start_pos, size_t& body_len) {


}

}


#include "base.h"
#include "instance.h"
#include <iostream>
#include <string>

namespace paxos {

Base::Base() {
    
}
    
Base::Base(const Instance* instance) {
    std::cout << "Base::Base()" << std::endl;
    this->instance = (Instance*)instance;
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

void Base::PackBaseMsg(const std::string& buf, const int cmd, std::string& str) {
    
}

int Base::PackMsg(const PaxosMsg& paxos_msg, std::string& str) {
    std::string buf;
    bool is_succeeded = paxos_msg.SerializeToString(&str);
    if (!is_succeeded) {
        return -1;
    }
    
    int cmd = 0;
    PackBaseMsg(buf, cmd, str);

    return 0;
}

int Base::UnpackBaseMsg(const std::string& str, Header& header, size_t& body_start_pos, size_t& body_len) {
    uint16_t header_len = 0;
    memcpy(&header_len, str.data() + GROUPIDXLEN, HEADLEN_LEN); 

    size_t header_start_pos = GROUPIDXLEN + HEADLEN_LEN;
    body_start_pos = header_start_pos + header_len;
    if (body_start_pos > str.size()) {
        // Throw an error
        return -1;
    }

    // Sth. I don't understand

    if (header.version() >= 1) {
        if (body_start_pos + CHECKSUM_LEN > str.size()) {
            // Throw an error
            return -1;
        }

        body_len = str.size() - CHECKSUM_LEN - body_start_pos;

        uint32_t buffer_check_sum = 0;
        memcpy(&buffer_check_sum, str.data() + str.size() - CHECKSUM_LEN, CHECKSUM_LEN);

        // Sth. related to crc32
    } 
    else {
        body_len = str.size() - body_start_pos;
    }

    return 0;
}

}


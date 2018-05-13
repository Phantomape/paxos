#include "base.h"
#include <iostream>
#include <string>

namespace paxos {

class Instance;

Base::Base() {
    
}
    
Base::Base(const Config* config, const Communicate* communicate, const Instance* instance) {
    config_ = (Config*)config;
    communicate_ = (Communicate*)communicate;
    instance_ = (Instance*)instance;

    instance_id_ = 0;

    is_test_mode_ = false;
}

Base::Base(const Instance* instance) {
    std::cout << "Base::Base()" << std::endl;
    instance_ = (Instance*)instance;
}

Base::~Base() {
    std::cout << "Base::~Base()" << std::endl;
}

int Base::BroadcastMessage(const PaxosMsg &paxos_msg, const int run_type, const int send_type) {
    std::cout << "Base::BroadcastMessage()" << std::endl;
    if (is_test_mode_) {
        return 0;
    }

    if (run_type == 0 && instance_->OnReceivePaxosMsg(paxos_msg) != 0) {
        return -1;
    }

    std::string str;
    int res = PackMsg(paxos_msg, str);
    if (res != 0) {
        return res;
    }

    // res = m_poMsgTransport->BroadcastMessage(m_poConfig->GetMyGroupIdx(), sBuffer, iSendType);

    if (run_type == 1) {
        instance_->OnReceivePaxosMsg(paxos_msg);
    }

    return res;
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


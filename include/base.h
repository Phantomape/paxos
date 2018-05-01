#pragma once

#include "ballot.h"
#include "paxos_msg.pb.h"

namespace paxos {

class Base {
public: 
    Base();
    virtual ~Base();

    virtual int BroadcastMessage(const PaxosMsg &paxos_msg);
    virtual void InitInstance() = 0;
    uint64_t GetInstanceId();
    void NewInstance();
    void SetInstanceId(const uint64_t instance_id);
    int UnpackBaseMsg(const std::string& str, Header& header, size_t& body_start_pos, size_t& body_len);
private:
    bool is_test_mode_;

    uint64_t instance_id_;
};

}
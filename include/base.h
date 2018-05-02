#pragma once

#include "ballot.h"
#include "paxos_msg.pb.h"

namespace paxos {

#define GROUPIDXLEN (sizeof(int))
#define HEADLEN_LEN (sizeof(uint16_t))
#define CHECKSUM_LEN (sizeof(uint32_t))

class Instance;

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

protected:
    Instance* instance;

private:
    bool is_test_mode_;

    uint64_t instance_id_;
};

}
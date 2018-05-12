#pragma once

#include "ballot.h"
#include "config.h"
#include "communicate.h"
#include "log_storage.h"
#include "paxos_msg.pb.h"

namespace paxos {

#define GROUPIDXLEN (sizeof(int))
#define HEADLEN_LEN (sizeof(uint16_t))
#define CHECKSUM_LEN (sizeof(uint32_t))

class Instance;

class Base {
public: 
    Base();
    Base(const Config * config, const Communicate * communicate, const Instance * instance);
    Base(const Instance* instance);
    virtual ~Base();

    virtual int BroadcastMessage(const PaxosMsg &paxos_msg, const int run_type, const int send_type);
    virtual void InitInstance() = 0;
    uint64_t GetInstanceId();
    void NewInstance();
    void PackBaseMsg(const std::string& buf, const int cmd, std::string& str);
    int PackMsg(const PaxosMsg& paxos_msg, std::string& str);
    void SetInstanceId(const uint64_t instance_id);
    int UnpackBaseMsg(const std::string& str, Header& header, size_t& body_start_pos, size_t& body_len);

protected:
    Instance* instance_;
    Config* config_;
    Communicate* communicate_;

private:
    bool is_test_mode_;

    uint64_t instance_id_;
};

}
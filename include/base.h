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

enum BroadcastMessage_Type
{
    BroadcastMessage_Type_RunSelf_First = 1,
    BroadcastMessage_Type_RunSelf_Final = 2,
    BroadcastMessage_Type_RunSelf_None = 3,
};

class Base {
public:
    Base(const Config* config, const Communicate* communicate, const Instance* instance);
    
    virtual ~Base();

    uint64_t GetInstanceId();

    void NewInstance();

    virtual void InitInstance() = 0;

    void SetInstanceId(const uint64_t instance_id);

    int PackMsg(const PaxosMsg& paxos_msg, std::string& buffer);
    
    int PackCheckpointMsg(const CheckpointMsg& checkpoint_msg, std::string& buffer);

    const uint32_t GetLastChecksum() const;
    
    void PackBaseMsg(const std::string& body_buffer, const int cmd, std::string & buffer);

    static int UnPackBaseMsg(const std::string & buffer, Header & header, size_t & body_start_pos, size_t & body_len);

    void SetAsTestMode();

protected:
    virtual int SendMessage(const uint64_t recv_node_id, const PaxosMsg& paxos_msg, const int send_type = Message_SendType_UDP);

    virtual int BroadcastMessage(
            const PaxosMsg& paxos_msg, 
            const int bRunSelfFirst = BroadcastMessage_Type_RunSelf_First,
            const int send_type = Message_SendType_UDP
            );
    
    int BroadcastMessageToFollower(
            const PaxosMsg& paxos_msg, 
            const int send_type = Message_SendType_TCP
            );
    
    int BroadcastMessageToTempNode(
            const PaxosMsg& paxos_msg, 
            const int send_type = Message_SendType_UDP
            );

    int SendMessage(const uint64_t recv_node_id, const CheckpointMsg& checkpoint_msg, 
            const int send_type = Message_SendType_TCP
            );

    Config * config_;
    Communicate * communicate_;
    Instance * instance_;

    uint64_t instance_id_;

    bool is_test_mode_;
};

}
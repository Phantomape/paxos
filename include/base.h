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
    Base(const Config * poConfig, const Communicate * poCommunicate, const Instance * poInstance);
    
    virtual ~Base();

    uint64_t GetInstanceId();

    void NewInstance();

    virtual void InitForNewPaxosInstance() = 0;

    void SetInstanceId(const uint64_t llInstanceId);

    int PackMsg(const PaxosMsg & oPaxosMsg, std::string & sBuffer);
    
    int PackCheckpointMsg(const CheckpointMsg & oCheckpointMsg, std::string & sBuffer);

public:
    const uint32_t GetLastChecksum() const;
    
    void PackBaseMsg(const std::string & sBodyBuffer, const int iCmd, std::string & sBuffer);

    static int UnPackBaseMsg(const std::string & sBuffer, Header & oHeader, size_t & iBodyStartPos, size_t & iBodyLen);

    void SetAsTestMode();

protected:
    virtual int SendMessage(const uint64_t iSendtoNodeId, const PaxosMsg & oPaxosMsg, const int iSendType = Message_SendType_UDP);

    virtual int BroadcastMessage(
            const PaxosMsg & oPaxosMsg, 
            const int bRunSelfFirst = BroadcastMessage_Type_RunSelf_First,
            const int iSendType = Message_SendType_UDP);
    
    int BroadcastMessageToFollower(
            const PaxosMsg & oPaxosMsg, 
            const int iSendType = Message_SendType_TCP);
    
    int BroadcastMessageToTempNode(
            const PaxosMsg & oPaxosMsg, 
            const int iSendType = Message_SendType_UDP);

protected:
    int SendMessage(const uint64_t iSendtoNodeId, const CheckpointMsg & oCheckpointMsg, 
            const int iSendType = Message_SendType_TCP);

protected:
    Config * config_;
    Communicate * communicate_;
    Instance * instance_;

    uint64_t instance_id_;

    bool is_test_mode_;
};

}
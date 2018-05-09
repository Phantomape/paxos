#pragma once

#include <vector>
#include "system_v_state_machine.h"

namespace paxos {

class Config {
public:
    Config(
        const LogStorage * poLogStorage,
        const bool bLogSync,
        const int iSyncInterval,
        const bool bUseMembership,
        const NodeInfo & oMyNode,
        const NodeInfoList & vecNodeInfoList,
        const FollowerNodeInfoList & vecFollowerNodeInfoList,
        const int iMyGroupIdx,
        const int iGroupCount,
        MembershipChangeCallback pMembershipChangeCallback);

    ~Config();

    int Init();

    const bool CheckConfig();

public:
    SystemVSM * GetSystemVSM();

public:
    const uint64_t GetGid() const;

    const uint64_t GetMyNodeID() const;
    
    const int GetNodeCount() const;

    const int GetMyGroupIdx() const;

    const int GetGroupCount() const;

    const int GetMajorityCount() const;

    const bool GetIsUseMembership() const;

public:
    const int GetPrepareTimeoutMs() const;

    const int GetAcceptTimeoutMs() const;

    const uint64_t GetAskforLearnTimeoutMs() const;

public:
    const bool IsValidNodeID(const uint64_t iNodeID);

    const bool IsIMFollower() const;

    const uint64_t GetFollowToNodeID() const;

    const bool LogSync() const;

    const int SyncInterval() const;

    void SetLogSync(const bool bLogSync);

public:
    void SetMasterSM(InternalStateMachine * poMasterSM);

    InternalStateMachine * GetMasterSM();

public:
    void AddTmpNodeOnlyForLearn(const uint64_t iTmpNodeID);

    //this function only for communicate.
    const std::map<uint64_t, uint64_t> & GetTmpNodeMap();

    void AddFollowerNode(const uint64_t iMyFollowerNodeID);

    //this function only for communicate.
    const std::map<uint64_t, uint64_t> & GetMyFollowerMap();

    const size_t GetMyFollowerCount();

private:
    bool m_bLogSync;
    int m_iSyncInterval;
    bool m_bUseMembership;

    uint64_t m_iMyNodeID;
    int m_iNodeCount;
    int m_iMyGroupIdx;
    int m_iGroupCount;

    NodeInfoList m_vecNodeInfoList;

    bool m_bIsIMFollower;
    uint64_t m_iFollowToNodeID;

    SystemVSM m_oSystemVSM;
    InternalStateMachine * m_poMasterSM;

    std::map<uint64_t, uint64_t> m_mapTmpNodeOnlyForLearn;
    std::map<uint64_t, uint64_t> m_mapMyFollower;
};

}

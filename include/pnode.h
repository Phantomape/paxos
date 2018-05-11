#pragma once

#include "default_network.h"
#include "node.h"
#include "notifier_pool.h"
#include "options.h"
#include "db.h"
#include "default_network.h"
#include "group.h"
#include "master_mgr.h"
#include "propose_batch.h"
#include "system_v_state_machine.h"
#include <vector>

namespace paxos
{

class PNode : public Node
{
public:
    PNode();

    ~PNode();

    int Init(const Options & oOptions, Network *& poNetwork);

public:
    int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID);
    int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, StateMachineCtx * poStateMachineCtx);
    const uint64_t GetCurrentInstanceId(const int iGroupIdx);
    const uint64_t GetMinChosenInstanceId(const int iGroupIdx);

public:
    //batch
    int BatchPropose(const int iGroupIdx, const std::string & sValue, 
            uint64_t & llInstanceID, uint32_t & iBatchIndex);
    int BatchPropose(const int iGroupIdx, const std::string & sValue, 
            uint64_t & llInstanceID, uint32_t & iBatchIndex, StateMachineCtx * poStateMachineCtx);
    void SetBatchCount(const int iGroupIdx, const int iBatchCount);
    void SetBatchDelayTimeMs(const int iGroupIdx, const int iBatchDelayTimeMs);

public:
    void AddStateMachine(StateMachine * poSM);
    void AddStateMachine(const int iGroupIdx, StateMachine * poSM);
    int OnReceiveMessage(const char * pcMessage, const int iMessageLen);
    const uint64_t GetNodeId() const;
    void SetTimeoutMs(const int iTimeoutMs);

public:
    void SetHoldPaxosLogCount(const uint64_t llHoldCount);
    void PauseCheckpointReplayer();
    void ContinueCheckpointReplayer();
    void PausePaxosLogCleaner();
    void ContinuePaxosLogCleaner();
    
public:
    //membership
    int AddMember(const int iGroupIdx, const NodeInfo & oNode);
    int RemoveMember(const int iGroupIdx, const NodeInfo & oNode);
    int ChangeMember(const int iGroupIdx, const NodeInfo & oFromNode, const NodeInfo & oToNode);
    int ShowMembership(const int iGroupIdx, NodeInfoList & vecNodeInfoList);

public:
    //master
    const NodeInfo GetMaster(const int iGroupIdx);
    const NodeInfo GetMasterWithVersion(const int iGroupIdx, uint64_t & llVersion);
    const bool IsIMMaster(const int iGroupIdx);
    int SetMasterLease(const int iGroupIdx, const int iLeaseTimeMs);
    int DropMaster(const int iGroupIdx);

public:
    void SetMaxHoldThreads(const int iGroupIdx, const int iMaxHoldThreads);
    void SetProposeWaitTimeThresholdMS(const int iGroupIdx, const int iWaitTimeThresholdMS);
    void SetLogSync(const int iGroupIdx, const bool bLogSync);

public:
    int GetInstanceValue(const int iGroupIdx, const uint64_t llInstanceID,
            std::vector<std::pair<std::string, int> > & vecValues);

private:
    int CheckOptions(const Options & oOptions);
    int InitLogStorage(const Options & oOptions, LogStorage *& poLogStorage);
    int InitNetwork(const Options & oOptions, Network *& poNetwork);
    int InitMaster(const Options & oOptions);
    void InitStateMachine(const Options & oOptions);
    bool CheckGroupID(const int iGroupIdx);
    int ProposalMembership(
            SystemVSM * poSystemVSM,
            const int iGroupIdx, 
            const NodeInfoList & vecNodeInfoList, 
            const uint64_t llVersion);

    void RunMaster(const Options & oOptions);
    void RunProposeBatch();

private:
    std::vector<Group *> m_vecGroupList;
    std::vector<MasterMgr *> m_vecMasterList;
    std::vector<ProposeBatch *> m_vecProposeBatch;

private:
    MultiDatabase m_oDefaultLogStorage;
    DefaultNetwork m_oDefaultNetwork;
    NotifierPool m_oNotifierPool;

    uint64_t m_iMyNodeID;
};
    
}

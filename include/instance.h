#pragma once

#include "acceptor.h"
#include "base.h"
#include "communicate.h"
#include "checkpoint_mgr.h"
#include "cleaner.h"
#include "commit_ctx.h"
#include "committer.h"
#include "ioloop.h"
#include "learner.h"
#include "proposer.h"
#include "replayer.h"
#include "state_machine.h"
#include "util.h"

#include <cstdint>

namespace paxos {

class Instance
{
public:
    Instance(
            const Config * poConfig, 
            const LogStorage * poLogStorage,
            const Communicate * poCommunicate,
            const Options & oOptions);
    ~Instance();

    int Init();

    void Start();

    void Stop();

    int InitLastCheckSum();

    const uint64_t GetInstanceId();

    const uint64_t GetMinChosenInstanceId();

    const uint32_t GetLastChecksum();

    int GetInstanceValue(const uint64_t llInstanceID, std::string & sValue, int & iSMID);

public:
    Committer * GetCommitter();

    Cleaner * GetCheckpointCleaner();

    Replayer * GetCheckpointReplayer();

public:
    void CheckNewValue();

    void OnNewValueCommitTimeout();

public:
    //this funciton only enqueue, do nothing.
    int OnReceiveMessage(const char * pcMessage, const int iMessageLen);

public:
    void OnReceive(const std::string & sBuffer);
    
    void OnReceiveCheckpointMsg(const CheckpointMsg & oCheckpointMsg);

    int OnReceivePaxosMsg(const PaxosMsg & oPaxosMsg, const bool bIsRetry = false);
    
    int ReceiveMsgForProposer(const PaxosMsg & oPaxosMsg);
    
    int ReceiveMsgForAcceptor(const PaxosMsg & oPaxosMsg, const bool bIsRetry);
    
    int ReceiveMsgForLearner(const PaxosMsg & oPaxosMsg);

public:
    void OnTimeout(const uint32_t iTimerID, const int iType);

public:
    void AddStateMachine(StateMachine * poSM);

    bool SMExecute(
        const uint64_t llInstanceID, 
        const std::string & sValue, 
        const bool bIsMyCommit,
        StateMachineCtx * poStateMachineCtx);

private:
    void ChecksumLogic(const PaxosMsg & oPaxosMsg);

    int PlayLog(const uint64_t llBeginInstanceID, const uint64_t llEndInstanceID);

    bool ReceiveMsgHeaderCheck(const Header & oHeader, const uint64_t iFromNodeID);

    int ProtectionLogic_IsCheckpointInstanceIDCorrect(const uint64_t llCPInstanceID, const uint64_t llLogMaxInstanceID);

    void NewInstance();

private:
    Config * m_poConfig;
    Communicate * m_poCommunicate;

    StateMachineFac m_oSMFac;

    IoLoop m_oIOLoop;

    Acceptor m_oAcceptor;
    Learner m_oLearner;
    Proposer m_oProposer;

    PaxosLog m_oPaxosLog;

    uint32_t m_iLastChecksum;

    CommitCtx m_oCommitCtx;
    uint32_t m_iCommitTimerID;

    Committer m_oCommitter;

    CheckpointMgr m_oCheckpointMgr;

    TimeStat m_oTimeStat;
    Options m_oOptions;

    bool m_bStarted;
};

}
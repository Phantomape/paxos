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

class Instance {
public:
    Instance(
            const Config* poConfig,
            const LogStorage* poLogStorage,
            const Communicate* poCommunicate,
            const Options& oOptions
            );

    ~Instance();

    int Init();

    void Start();

    void Stop();

    int InitLastChecksum();

    const uint64_t GetInstanceId();

    const uint64_t GetMinChosenInstanceId();

    const uint32_t GetLastChecksum();

    int GetInstanceValue(const uint64_t instance_id, std::string& sValue, int& iSMID);

    Committer* GetCommitter();

    Cleaner* GetCheckpointCleaner();

    Replayer* GetCheckpointReplayer();

    void CheckNewValue();

    void OnNewValueCommitTimeout();

    int OnReceiveMessage(const char* pcMessage, const int iMessageLen);

    void OnReceive(const std::string& sBuffer);

    void OnReceiveCheckpointMsg(const CheckpointMsg& oCheckpointMsg);

    int OnReceivePaxosMsg(const PaxosMsg& paxos_msg, const bool bIsRetry = false);

    int ReceiveMsgForProposer(const PaxosMsg& paxos_msg);

    int ReceiveMsgForAcceptor(const PaxosMsg& paxos_msg, const bool bIsRetry);

    int ReceiveMsgForLearner(const PaxosMsg& paxos_msg);

    void OnTimeout(const uint32_t iTimerID, const int iType);

    void AddStateMachine(StateMachine* poSM);

    bool SMExecute(
        const uint64_t instance_id,
        const std::string& sValue,
        const bool bIsMyCommit,
        StateMachineCtx* poStateMachineCtx);

private:
    void ChecksumLogic(const PaxosMsg& paxos_msg);

    int PlayLog(const uint64_t llBeginInstanceID, const uint64_t llEndInstanceID);

    bool ReceiveMsgHeaderCheck(const Header& oHeader, const uint64_t iFromNodeID);

    int ProtectionLogic_IsCheckpointInstanceIDCorrect(const uint64_t llCPInstanceID, const uint64_t llLogMaxInstanceID);

    void NewInstance();

    Config* config_;

    Communicate* communicate_;

    StateMachineFac state_machine_fac_;

    IoLoop ioloop_;

    Acceptor acceptor_;
    Learner learner_;
    Proposer proposer_;

    PaxosLog paxos_log_;

    uint32_t last_checksum_;

    CommitCtx commit_ctx_;
    uint32_t commit_timer_id_;

    Committer commiter_;

    CheckpointMgr checkpoint_mgr_;

    TimeStat time_stat_;
    Options options_;

    bool is_started_;
};

}
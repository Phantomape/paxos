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
            const Config * poConfig, 
            const LogStorage * poLogStorage,
            const Communicate * poMsgTransport,
            const Options & oOptions
            );

    ~Instance();

    void AddStateMachine(StateMachine * state_machine);

    void CheckNewValue();

    int ForwardToAcceptor(const PaxosMsg& paxos_msg, const bool is_retry);
    
    int ForwardToLearner(const PaxosMsg& paxos_msg);

    int ForwardToProposer(const PaxosMsg& paxos_msg);

    Committer * GetCommitter();

    Cleaner * GetCheckpointCleaner();

    Replayer * GetCheckpointReplayer();

    int GetInstanceValue(const uint64_t instance_id, std::string & val, int & state_machine_id);

    int Init();
    
    int OnReceiveMessage(const char * msg, const int msg_len);

    void Start();
    void Stop();

    const uint64_t GetCurrentInstanceId();
    
    const uint64_t GetMinInstanceId();

    void OnReceive(const std::string& str);

    int OnReceivePaxosMsg(const PaxosMsg& paxos_msg, const bool should_retry = false);
private:
    Acceptor acceptor_;

    IoLoop ioloop_;
    
    Learner learner_;
    
    Proposer proposer_;

    Config * config_;

    Communicate * communicate_;

    StateMachineFac state_machine_fac_;

    PaxosLog paxos_log_;

    uint32_t last_checksum_;

    CommitCtx commit_ctx_;

    uint32_t commit_timer_id_;

    Committer committer_;

    CheckpointMgr checkpoint_mgr_;

    TimeStat m_oTimeStat;
    
    Options m_oOptions;

    bool m_bStarted;

    bool is_started_;
};

}
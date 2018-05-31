#pragma once

#include "base.h"
#include "msg_counter.h"
#include "ioloop.h"
#include "ballot.h"

namespace paxos {

class Learner;

class Proposer : public Base {
public:
    Proposer(
            const Config* config, 
            const Communicate * communicate,
            const Instance * poInstance,
            const Learner * poLearner,
            const IoLoop * poIOLoop
            );

    ~Proposer();

    void Accept();

    void AddPrepareTimer(const int timeout_ms = 0);
    
    void AddAcceptTimer(const int timeout_ms = 0);

    void Prepare(const bool need_new_ballot);

    void ExitPrepare();

    void ExitAccept();

    const uint64_t GetProposalId();
    const std::string& GetValue();

    virtual void InitInstance();

    void OnAccept(const PaxosMsg &paxos_msg);
    void OnAcceptRejected(const PaxosMsg &paxos_msg);
    void OnAcceptTimeout();
    void OnPrepare(const PaxosMsg &paxos_msg);
    void OnPrepareRejected(const PaxosMsg &paxos_msg);
    void OnPrepareTimeout();

    // Generate a new value to propose
    void Propose(const std::string& val);

    void SetValue(const std::string& val);

    void UpdateOtherProposalId(const uint64_t other_proposal_id);

private:

    bool is_preparing_;
    bool is_accepting_;
    bool is_rejected_;
    bool can_skip_prepare_;
    bool rejected_;

    Config* config_;

    std::string val_;

    uint64_t proposal_id_;
    uint64_t highest_proposal_id_by_others_;

    uint32_t prepare_timer_id_;
    int last_prepare_timeout_ms_;
    uint32_t accept_timer_id_;
    int last_accept_timeout_ms_;
    uint64_t timeout_instance_id_;

    Learner* learner_;
    IoLoop* ioloop_;

    Ballot highest_other_pre_accept_ballot_;
    MsgCounter msg_counter; // Fully defined before owner class???s

    TimeStat time_stat_;
};

}
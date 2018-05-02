#pragma once

#include "base.h"
#include "msg_counter.h"
#include "ballot.h"

namespace paxos {

class Proposer : public Base {
public:
    Proposer(const Instance* instance);
    ~Proposer();

    void Accept();
    void Prepare();

    void ExitPrepare();
    void ExitAccept();

    virtual void InitInstance();

    void OnAccept(const PaxosMsg &paxos_msg);
    void OnAcceptRejected(const PaxosMsg &paxos_msg);
    void OnAcceptTimeout();
    void OnPrepare(const PaxosMsg &paxos_msg);
    void OnPrepareRejected(const PaxosMsg &paxos_msg);
    void OnPrepareTimeout();

    // Generate a new value to propose
    void Propose();

    void UpdateOtherProposalId(const uint64_t other_proposal_id);
private:
    bool is_preparing_;
    bool is_accepting_;
    bool is_rejected_;
    bool can_skip_prepare_;
    bool rejected_;

    std::string val_;

    uint64_t proposal_id_;
    uint64_t highest_proposal_id_by_others_;
    uint64_t timeout_instance_id_;

    Ballot highest_other_pre_accept_ballot_;
    MsgCounter msg_counter; // Fully defined before owner class???s
};

}
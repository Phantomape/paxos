#pragma once

#include "base.h"
#include "msg_counter.h"
#include "ballot.h"

namespace paxos {

class Proposer : public Base {
public:
    Proposer();
    ~Proposer();

    void Accept();
    void Prepare();

    void ExitPrepare();
    void ExitAccept();

    void OnAccept();
    void OnAcceptRejected();
    void OnAcceptTimeout();
    void OnPrepare(const PaxosMsg &paxos_msg);
    void OnPrepareRejected();
    void OnPrepareTimeout();

    // Generate a new value to propose
    void Propose();
private:
    bool is_preparing_;
    bool is_accepting_;
    bool is_rejected_;
    bool can_skip_prepare_;
    bool rejected_;

    std::string val_;

    uint64_t proposal_id_;

    Ballot highest_other_pre_accept_ballot_;
    MsgCounter msg_counter; // Fully defined before owner class???s
};

}
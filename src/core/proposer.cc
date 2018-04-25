#include "proposer.h"
#include "ballot.h"
#include <iostream>
#include <string>

namespace paxos {
    
Proposer::Proposer() {
    std::cout << "Proposer::Proposer()" << std::endl;
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

Proposer::~Proposer() {
    std::cout << "Proposer::~Proposer()" << std::endl;
}

void Proposer::Accept() {
    std::cout << "Proposer::Accept()" << std::endl;
    is_accepting_ = true;
}

void Proposer::Prepare() {
    std::cout << "Proposer::Prepare()" << std::endl;

    ExitAccept();
    is_preparing_ = true;
    can_skip_prepare_ = false;
    rejected_ = false;

    highest_other_pre_accept_ballot_.reset();
    bool need_new_ballot = false;
    if (need_new_ballot) {
        // No idea
    }

    // Calculate votes
    PaxosMsg send_paxos_msg;
    send_paxos_msg.set_msgtype(1);   // Replace 1 with some enum

    BroadcastMessage(send_paxos_msg);
}

void Proposer::ExitPrepare() {}
void Proposer::ExitAccept() {
    if (is_accepting_) {
        is_accepting_ = false;
    }
}

void Proposer::OnAccept() {

}

void Proposer::OnAcceptRejected() {}
void Proposer::OnAcceptTimeout() {}

void Proposer::OnPrepare(const PaxosMsg &recv_paxos_msg) {
    if (!is_preparing_) {
        // Skip this msg if not preparing or id differs
        return;
    }

    // Ignore those with different proposal_id
    if (recv_paxos_msg.proposalid() != proposal_id_) {
        return;
    }

    // Received a message
    msg_counter.AddReceivedMsg();
    
    if (true) {// Accepted
        msg_counter.AddAcceptedMsg();
    } 
    else {
        is_rejected_ = true;
        msg_counter.AddRejectedMsg();
    }

    if (msg_counter.IsPassed()) {
        can_skip_prepare_ = true;

        Accept();
    } 
    else if (msg_counter.IsRejected()) {

    }
}

void Proposer::OnPrepareRejected() {}
void Proposer::OnPrepareTimeout() {}

void Proposer::Propose() {
    // set the value
    
    // check whether it can skip prepare without rejection
    // else goes into Prepare()
    if (can_skip_prepare_) {
        Accept();
    }
    else {
        Prepare();
    }
}

}
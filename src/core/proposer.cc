#include "proposer.h"
#include "ballot.h"
#include <iostream>
#include <string>

namespace paxos {
    
Proposer::Proposer() {
    std::cout << "Proposer::Proposer()" << std::endl;
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    is_accepting_ = false;
    is_preparing_ = false;

    can_skip_prepare_ = false;

    InitInstance();

    is_rejected_ = false;
}

Proposer::~Proposer() {
    std::cout << "Proposer::~Proposer()" << std::endl;
}

void Proposer::Accept() {
    std::cout << "Proposer::Accept()" << std::endl;
    ExitPrepare();
    is_accepting_ = true;

    PaxosMsg paxos_msg;
    paxos_msg.set_msgtype(1);

    msg_counter.Init();

    BroadcastMessage(paxos_msg);
}

void Proposer::ExitAccept() {
    if (is_accepting_) {
        is_accepting_ = false;
    }
}

void Proposer::ExitPrepare() {
    if (is_preparing_) {
        is_preparing_ = false;
    }
}

void Proposer::InitInstance() {
    // Start a new round for msg_counter
    msg_counter.Init();

    val_.clear();
    highest_proposal_id_by_others_ = 0;

    ExitPrepare();
    ExitAccept();
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
        // Init new proposer state
        proposal_id_ = std::max(proposal_id_, highest_proposal_id_by_others_) + 1;
    }

    // Calculate votes
    PaxosMsg send_paxos_msg;
    send_paxos_msg.set_msgtype(1);   // Replace 1 with some enum

    msg_counter.Init();

    BroadcastMessage(send_paxos_msg);
}

void Proposer::OnAccept(const PaxosMsg &paxos_msg) {
    if (!is_accepting_) {
        return;
    }

    if (paxos_msg.proposalid() != proposal_id_) {
        return;
    }

    msg_counter.AddReceivedMsg(paxos_msg.nodeid());

    if (paxos_msg.rejectbypromiseid() == 0) {
        msg_counter.AddAcceptedMsg(paxos_msg.nodeid());
    }
    else {
        msg_counter.AddReceivedMsg(paxos_msg.nodeid());
        is_rejected_ = true;
        UpdateOtherProposalId(paxos_msg.rejectbypromiseid());
    }

    if (msg_counter.IsPassed()) {
        can_skip_prepare_ = true;
        Accept();
    }
    else if (msg_counter.IsRejected() || msg_counter.IsAllReceived()) {
        // Do sth. I don't understand
    }
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
    msg_counter.AddReceivedMsg(recv_paxos_msg.nodeid());
    
    if (true) {// Accepted
        msg_counter.AddAcceptedMsg(recv_paxos_msg.nodeid());
    } 
    else {
        is_rejected_ = true;
        msg_counter.AddRejectedMsg(recv_paxos_msg.nodeid());
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

void Proposer::UpdateOtherProposalId(const uint64_t other_proposal_id) {
    if (other_proposal_id > highest_proposal_id_by_others_) {
        highest_proposal_id_by_others_ = other_proposal_id;
    }
}

}
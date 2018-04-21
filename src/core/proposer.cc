#include "proposer.h"
#include "test_paxos_msg.pb.h"
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

    is_preparing_ = true;
    can_skip_prepare_ = false;
    rejected_ = false;

    bool need_new_ballot = false;
    if (need_new_ballot) {
        // No idea
    }

    // Calculate votes

    BroadcastMessage();
}

void Proposer::ExitPrepare() {}
void Proposer::ExitAccept() {}

void Proposer::OnAccept() {

}

void Proposer::OnAcceptRejected() {}
void Proposer::OnAcceptTimeout() {}

void Proposer::OnPrepare() {
    if (!is_preparing_) {
        // Skip this msg if not preparing or id differs
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
}

}
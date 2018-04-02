#include "proposer.h"
#include <iostream>
#include <string>

namespace paxos {
    
Proposer::Proposer() {
    std::cout << "Proposer::Proposer()" << std::endl;
}

Proposer::~Proposer() {
    std::cout << "Proposer::~Proposer()" << std::endl;
}

void Proposer::Accept() {
    std::cout << "Proposer::Accept()" << std::endl;
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

void Proposer::OnAccept() {}
void Proposer::OnAcceptRejected() {}
void Proposer::OnAcceptTimeout() {}

void Proposer::OnPrepare() {
    if (!is_preparing_) {
        // Skip this msg if not preparing or id differs
        return;
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
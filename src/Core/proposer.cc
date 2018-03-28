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

void Propose::Accept() {
    std::cout << "Proposer::Accept()" << std::endl;
}

void Proposer::Prepare() {
    std::cout << "Proposer::Prepare()" << std::endl;
}

void Proposer::ExitPrepare() {}
void Proposer::ExitAccept() {}

void Proposer::OnAccept() {}
void Proposer::OnAcceptRejected() {}
void Proposer::OnAcceptTimeout() {}
void Proposer::OnPrepare() {}
void Proposer::OnPrepareRejected() {}
void Proposer::OnPrepareTimeout() {}
}
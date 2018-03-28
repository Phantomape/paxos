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

void Proposer::Prepare() {
    std::cout << "Proposer::Prepare()" << std::endl;
}

void Propose::Accept() {
    std::cout << "Proposer::Accept()" << std::endl;
}

}
#include "base.h"
#include "proposer.h"

int main() {
    paxos::Base* base = new paxos::Base();
    delete base;

    paxos::Proposer* proposer = new paxos::Proposer();
    proposer->Prepare();
    delete proposer;
    return 0;
}
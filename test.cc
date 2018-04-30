#include "base.h"
#include "proposer.h"

int main() {
    paxos::Proposer* proposer = new paxos::Proposer();
    proposer->Prepare();
    delete proposer;
    return 0;
}
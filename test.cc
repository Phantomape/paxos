#include "base.h"

int main() {
    paxos::Base* base;
    base = new paxos::Base();
    delete base;
    return 0;
}
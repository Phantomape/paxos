#pragma once

#include "base.h"

namespace paxos {

class Proposer : public Base {
public:
    Proposer();
    ~Proposer();
    void Prepare();
    void Accept();
};

}
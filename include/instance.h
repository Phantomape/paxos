#pragma once

#include "acceptor.h"
#include "learner.h"
#include "proposer.h"

#include <cstdint>

namespace paxos {

class Instance {
public:
    Instance();
    ~Instance();

    int Init();
    void Start();
    void Stop();

    const uint64_t GetCurrentInstanceId();
    const uint64_t GetMinInstanceId();
private:
    Acceptor acceptor;
    Learner learner;
    Proposer proposer;

    bool is_started_;
};

}
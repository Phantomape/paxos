#pragma once

#include "acceptor.h"
#include "base.h"
#include "ioloop.h"
#include "learner.h"
#include "proposer.h"

#include <cstdint>

namespace paxos {

class Instance {
public:
    Instance();
    ~Instance();

    void CheckNewValue();

    int ForwardToAcceptor(const PaxosMsg& paxos_msg, const bool is_retry);
    int ForwardToLearner(const PaxosMsg& paxos_msg);
    int ForwardToProposer(const PaxosMsg& paxos_msg);

    int Init();
    void Start();
    void Stop();

    const uint64_t GetCurrentInstanceId();
    const uint64_t GetMinInstanceId();

    void OnReceive(const std::string& str);
    int OnReceivePaxosMsg(const PaxosMsg& paxos_msg, const bool should_retry = false);
private:
    Acceptor acceptor;
    //IoLoop ioloop;
    Learner learner;
    Proposer proposer;

    bool is_started_;
};

}
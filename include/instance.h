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

    int ForwardToAcceptor(const PaxosMsg& paxos_msg);
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
    Learner learner;
    Proposer proposer;

    bool is_started_;
};

}
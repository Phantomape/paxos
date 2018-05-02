#include "instance.h"

namespace paxos {

Instance::Instance() {}

Instance::~Instance() {}

void Instance::CheckNewValue() {
    
}

int Instance::ForwardToAcceptor(const PaxosMsg& paxos_msg) {
    std::cout << "Instance::ForwardToAcceptor()" << std::endl;
    return 0;
}

int Instance::ForwardToLearner(const PaxosMsg& paxos_msg) {
    std::cout << "Instance::ForwardToLearner()" << std::endl;
    return 0;
}

int Instance::ForwardToProposer(const PaxosMsg& paxos_msg) {
    std::cout << "Instance::ForwardToProposer()" << std::endl;
    return 0;
}

int Instance::Init() {
    std::cout << "Instance::Init()" << std::endl;
    int res = acceptor.Init();
    if (res != 0) {
        return res;
    }

    // Sth. related to check point
    uint64_t CurrentInstanceId = 0;
    if (CurrentInstanceId < acceptor.GetInstanceId()) {
        CurrentInstanceId = acceptor.GetInstanceId();
    }
    else {
        if (CurrentInstanceId > acceptor.GetInstanceId()) {
            // Do sth.
            acceptor.InitInstance();
        }

        acceptor.SetInstanceId(CurrentInstanceId);
    }

    learner.SetInstanceId(CurrentInstanceId);
    proposer.SetInstanceId(CurrentInstanceId);

    // Sth. I don't understand
    return 0;
}

void Instance::OnReceive(const std::string& str) {
    if (str.size() <= 6) {
        // Throw an error
        return;
    }

    size_t body_start_pos = 0, body_len = 0;
    // Unpack this basic message and call different function
}

int Instance::OnReceivePaxosMsg(const PaxosMsg& paxos_msg, const bool should_retry) {
    if (false) {// Msg types for proposer
        // Check whether node id is valid
        return ForwardToProposer(paxos_msg);
    }
    else if (false) {
        return ForwardToAcceptor(paxos_msg);
    }
    else if (false) {
        return ForwardToLearner(paxos_msg);
    }
    else {
        // Throw invalid msg type error
    }
    return 0;
}

void Instance::Start() {
    std::cout << "Instance::Start()" << std::endl;
    learner.InitLearnerSynchronizer();
    // Start ioloop ycheckpoint
    is_started_ = true;
}

void Instance::Stop() {
    std::cout << "Instance::Stop()" << std::endl;
    if (is_started_) {
        // Stop ioloop and checkpoint
        learner.Stop();
    }
}

}

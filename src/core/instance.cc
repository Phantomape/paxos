#include "instance.h"
#include "acceptor.h"
#include "learner.h"
#include "proposer.h"

namespace paxos {

Instance::Instance() : acceptor(this), learner(this, &acceptor), proposer(this) {

}

Instance::~Instance() {}

void Instance::CheckNewValue() {

}

int Instance::ForwardToAcceptor(const PaxosMsg& paxos_msg, const bool is_retry) {
    std::cout << "Instance::ForwardToAcceptor()" << std::endl;
    // Sth. related to IsImFollower

    if (paxos_msg.instanceid() != acceptor.GetInstanceId()) {
    
    }

    if (paxos_msg.instanceid() == acceptor.GetInstanceId() + 1) {
        PaxosMsg send_paxos_msg = paxos_msg;
        send_paxos_msg.set_instanceid(acceptor.GetInstanceId());
        send_paxos_msg.set_msgtype(1);

        ForwardToLearner(send_paxos_msg);
    }

    if (paxos_msg.instanceid() == acceptor.GetInstanceId()) {
        if (paxos_msg.msgtype() == 1) {
            return acceptor.OnPrepare(paxos_msg);
        }
        else if (paxos_msg.msgtype() == 2) {
            acceptor.OnAccept(paxos_msg);
        }
    }
    else if (is_retry == false && paxos_msg.instanceid() > acceptor.GetInstanceId()) {
        if (paxos_msg.instanceid() >= learner.GetLatestInstanceID()) {
            if (paxos_msg.instanceid() < acceptor.GetInstanceId() + RETRY_QUEUE_MAX_LEN) {
                //ioloop.AddRetryPaxosMsg(paxos_msg);
            }
        }
        else {
            //ioloop.ClearRetryQueue();
        }
    }
        
    return 0;
}

int Instance::ForwardToLearner(const PaxosMsg& paxos_msg) {
    std::cout << "Instance::ForwardToLearner()" << std::endl;
    return 0;
}

int Instance::ForwardToProposer(const PaxosMsg& paxos_msg) {
    std::cout << "Instance::ForwardToProposer()" << std::endl;
    // Sth. related to IsImFollower

    if (paxos_msg.instanceid() != proposer.GetInstanceId()) {
        if (paxos_msg.instanceid() + 1 == proposer.GetInstanceId()) {
            if (paxos_msg.msgtype() == 1) {
                proposer.OnPrepareRejected(paxos_msg);
            }
            else if (paxos_msg.msgtype() == 2) {
                proposer.OnAcceptRejected(paxos_msg);
            }
        }

        return 0;
    }

    if (paxos_msg.msgtype() == 3) {
        proposer.OnPrepare(paxos_msg);
    }
    else if (paxos_msg.msgtype() == 4) {
        proposer.OnAccept(paxos_msg);
    }

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
        return ForwardToAcceptor(paxos_msg, should_retry);
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

#include "acceptor.h"
#include "ballot.h"
#include <iostream>
#include <string>

namespace paxos {

Acceptor::Acceptor(
        const Config* config,
        const Communicate* communicate,
        const Instance* instance,
        const LogStorage* log_storage)
    : Base(config, communicate, instance), paxos_log_(log_storage) {
    config_ = (Config*)config;
}


Acceptor::~Acceptor() {
}

int Acceptor::Init() {
    uint64_t instance_id = 0;
    int res = Load(instance_id);
    if (res != 0) {
        return res;
    }

    if (instance_id == 0) {
        std::cout << "Empty database" << std::endl;
    }

    SetInstanceId(instance_id);
    // Not sure why we need a return value of integer
    return 0;
}

void Acceptor::InitInstance() {
    accepted_ballot_.reset();
    accepted_val_ = "";
}

const Ballot& Acceptor::GetAcceptedBallot() const {
    std::cout << "Acceptor::GetAcceptedBallot()" << std::endl;
    return accepted_ballot_;
}

const std::string& Acceptor::GetAcceptedValue() {
    std::cout << "Acceptor::GetAcceptredValue()" << std::endl;
    return accepted_val_;
}

const Ballot& Acceptor::GetPromiseBallot() const {
    std::cout << "Acceptor::GetPromiseBallot()" << std::endl;
    return promised_ballot_;
}

int Acceptor::Load(uint64_t & instance_id) {
    int ret = paxos_log_.GetMaxInstanceIDFromLog(config_->GetMyGroupIdx(), instance_id);
    if (ret != 0 && ret != 1) {
        return ret;
    }

    if (ret == 1) {
        instance_id = 0;
        return 0;
    }

    AcceptorStateData state;
    ret = paxos_log_.ReadState(config_->GetMyGroupIdx(), instance_id, state);
    if (ret != 0) {
        return ret;
    }

    promised_ballot_.proposal_id_ = state.promiseid();
    promised_ballot_.node_id_ = state.promisenodeid();
    accepted_ballot_.proposal_id_ = state.acceptedid();
    accepted_ballot_.node_id_ = state.acceptednodeid();
    accepted_val_ = state.acceptedvalue();
    checksum_ = state.checksum();

    return 0;
}

void Acceptor::SetAcceptedBallot(const Ballot& accepted_ballot) {
    std::cout << "Acceptor::SetAcceptedBallot()" << std::endl;
    this->accepted_ballot_ = accepted_ballot;
}

void Acceptor::SetAcceptedValue(const std::string& accepted_val) {
    std::cout << "Acceptor::SetAcceptedValue()" << std::endl;
    accepted_val_ = accepted_val;
}

void Acceptor::SetPromiseBallot(const Ballot& promised_ballot) {
    std::cout << "Acceptor::SetPromiseBallot()" << std::endl;
    this->promised_ballot_ = promised_ballot;
}

int Acceptor::OnPrepare(const PaxosMsg &recv_paxos_msg) {
    std::cout << "Acceptor::OnPrepare()" << std::endl;

    PaxosMsg send_paxos_msg;
    send_paxos_msg.set_msgtype(1); // Replace thie hard-coded number

    Ballot ballot(recv_paxos_msg.proposalid(), recv_paxos_msg.nodeid());
    if (ballot >= GetPromiseBallot()) {
        send_paxos_msg.set_preacceptid(GetAcceptedBallot().proposal_id_);
        send_paxos_msg.set_preacceptnodeid(GetAcceptedBallot().node_id_);

        if (GetAcceptedBallot().proposal_id_ > 0) {
            send_paxos_msg.set_value(GetAcceptedValue());
        }
        SetPromiseBallot(ballot);
    }
    else {
        send_paxos_msg.set_rejectbypromiseid(GetPromiseBallot().proposal_id_);
    }

    uint64_t send_node_id = recv_paxos_msg.nodeid();
    //SendMessage(send_node_id, send_paxos_msg);

    return 0;
}

void Acceptor::OnAccept(const PaxosMsg &paxos_msg) {
    std::cout << "Acceptor::Accept()" << std::endl;

    PaxosMsg send_paxos_msg;
    send_paxos_msg.set_instanceid(GetInstanceId());

    Ballot ballot(paxos_msg.proposalid(), paxos_msg.nodeid());
}

}

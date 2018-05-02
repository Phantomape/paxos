#include "acceptor.h"
#include "ballot.h"
#include <iostream>
#include <string>

namespace paxos {
    
Acceptor::Acceptor(const Instance* instance) : Base(instance){
    std::cout << "Acceptor::Acceptor()" << std::endl;
}

Acceptor::~Acceptor() {
    std::cout << "Acceptor::~Acceptor()" << std::endl;
}

int Acceptor::Init() {
    uint64_t instance_id = 0;
    int res = 1;
    if (res != 0) {

    }

    if (instance_id == 0) {

    }

    SetInstanceId(instance_id);
    // Not sure why we need a return value of integer
    return 0;
}

void Acceptor::InitInstance() {
    accepted_ballot.reset();
    accepted_val_ = "";
}

const Ballot& Acceptor::GetAcceptedBallot() const {
    std::cout << "Acceptor::GetAcceptedBallot()" << std::endl;
    return accepted_ballot;
}

const std::string& Acceptor::GetAcceptedValue() {
    std::cout << "Acceptor::GetAcceptredValue()" << std::endl;
    return accepted_val_;   
}

const Ballot& Acceptor::GetPromiseBallot() const {
    std::cout << "Acceptor::GetPromiseBallot()" << std::endl;
    return promised_ballot;
}

void Acceptor::SetAcceptedBallot(const Ballot& accepted_ballot) {
    std::cout << "Acceptor::SetAcceptedBallot()" << std::endl;
    this->accepted_ballot = accepted_ballot;
}

void Acceptor::SetAcceptedValue(const std::string& accepted_val) {
    std::cout << "Acceptor::SetAcceptedValue()" << std::endl;
    accepted_val_ = accepted_val;
}

void Acceptor::SetPromiseBallot(const Ballot& promised_ballot) {
    std::cout << "Acceptor::SetPromiseBallot()" << std::endl;
    this->promised_ballot = promised_ballot;
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


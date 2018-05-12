#include "instance.h"
#include "proposer.h"
#include <iostream>
#include <string>

namespace paxos {
    
Proposer::Proposer(
        const Config * poConfig, 
        const Communicate * poMsgTransport,
        const Instance * poInstance,
        const Learner * poLearner,
        const IoLoop * poIOLoop)
    : Base(poConfig, poMsgTransport, poInstance)
{}

Proposer::Proposer(const Instance* instance) : Base(instance) {
    std::cout << "Proposer::Proposer()" << std::endl;
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    is_accepting_ = false;
    is_preparing_ = false;

    can_skip_prepare_ = false;

    InitInstance();

    timeout_instance_id_ = 0;

    is_rejected_ = false;
}

Proposer :: Proposer(
        const Config * config, 
        const Communicate * communicate,
        const Instance * poInstance,
        const Learner * poLearner,
        const IOLoop * poIOLoop)
    : Base(config, communicate, poInstance), m_oMsgCounter(config)
{
    config_ = (Config*)config;
    m_poLearner = (Learner *)poLearner;
    m_poIOLoop = (IOLoop *)poIOLoop;
    
    m_bIsPreparing = false;
    m_bIsAccepting = false;

    m_bCanSkipPrepare = false;

    InitForNewPaxosInstance();

    m_iPrepareTimerID = 0;
    m_iAcceptTimerID = 0;
    m_llTimeoutInstanceID = 0;

    m_iLastPrepareTimeoutMs = m_poConfig->GetPrepareTimeoutMs();
    m_iLastAcceptTimeoutMs = m_poConfig->GetAcceptTimeoutMs();

    m_bWasRejectBySomeone = false;
}

Proposer::~Proposer() {
    std::cout << "Proposer::~Proposer()" << std::endl;
}

void Proposer::Accept() {
    std::cout << "Proposer::Accept()" << std::endl;
    ExitPrepare();
    is_accepting_ = true;

    PaxosMsg paxos_msg;
    paxos_msg.set_msgtype(1);
    paxos_msg.set_instanceid(GetInstanceId());
    paxos_msg.set_proposalid(GetProposalId());
    paxos_msg.set_value(GetValue());

    msg_counter.Init();

    BroadcastMessage(paxos_msg, 1, 1);
}

void Proposer::ExitAccept() {
    if (is_accepting_) {
        is_accepting_ = false;
    }
}

void Proposer::ExitPrepare() {
    if (is_preparing_) {
        is_preparing_ = false;
    }
}

const uint64_t Proposer::GetProposalId() {
    return proposal_id_;
}

const std::string& Proposer::GetValue() {
    return val_;
}

void Proposer::InitInstance() {
    // Start a new round for msg_counter
    msg_counter.Init();

    val_.clear();
    highest_proposal_id_by_others_ = 0;

    ExitPrepare();
    ExitAccept();
}

// This method is where proposer would learn what value to propose, if
// not value is chosen in this stage, they can propose their own value
void Proposer::Prepare() {
    std::cout << "Proposer::Prepare()" << std::endl;

    ExitAccept();
    is_preparing_ = true;
    can_skip_prepare_ = false;
    rejected_ = false;

    highest_other_pre_accept_ballot_.reset();
    bool need_new_ballot = false;
    if (need_new_ballot) {
        // Init new proposer state
        proposal_id_ = std::max(proposal_id_, highest_proposal_id_by_others_) + 1;
    }

    // Calculate votes
    PaxosMsg send_paxos_msg;
    send_paxos_msg.set_msgtype(1);   // Replace 1 with some enum
    send_paxos_msg.set_instanceid(GetInstanceId());
    send_paxos_msg.set_proposalid(GetProposalId());

    msg_counter.Init();

    BroadcastMessage(send_paxos_msg, 1, 1);
}

void Proposer::OnAccept(const PaxosMsg &paxos_msg) {
    if (!is_accepting_) {
        return;
    }

    if (paxos_msg.proposalid() != proposal_id_) {
        return;
    }

    msg_counter.AddReceivedMsg(paxos_msg.nodeid());

    if (paxos_msg.rejectbypromiseid() == 0) {
        msg_counter.AddAcceptedMsg(paxos_msg.nodeid());
    }
    else {
        msg_counter.AddReceivedMsg(paxos_msg.nodeid());
        is_rejected_ = true;
        UpdateOtherProposalId(paxos_msg.rejectbypromiseid());
    }

    if (msg_counter.IsPassed()) {
        can_skip_prepare_ = true;
        Accept();
    }
    else if (msg_counter.IsRejected() || msg_counter.IsAllReceived()) {
        // Do sth. I don't understand
    }
}

void Proposer::OnAcceptRejected(const PaxosMsg &paxos_msg) {
    if (paxos_msg.rejectbypromiseid() != 0) {
        is_rejected_ = true;
        UpdateOtherProposalId(paxos_msg.rejectbypromiseid());
    }
}

void Proposer::OnAcceptTimeout() {
    if (GetInstanceId() != timeout_instance_id_) {
        return;
    }
    Prepare();
}

void Proposer::OnPrepare(const PaxosMsg &recv_paxos_msg) {
    if (!is_preparing_) {
        // Skip this msg if not preparing or id differs
        return;
    }

    // Ignore those with different proposal_id
    if (recv_paxos_msg.proposalid() != proposal_id_) {
        return;
    }

    // Received a message
    msg_counter.AddReceivedMsg(recv_paxos_msg.nodeid());
    
    if (true) {// Accepted
        msg_counter.AddAcceptedMsg(recv_paxos_msg.nodeid());
    } 
    else {
        is_rejected_ = true;
        msg_counter.AddRejectedMsg(recv_paxos_msg.nodeid());
    }

    if (msg_counter.IsPassed()) {
        can_skip_prepare_ = true;

        Accept();
    } 
    else if (msg_counter.IsRejected()) {

    }
}

void Proposer::OnPrepareRejected(const PaxosMsg &paxos_msg) {
    if (paxos_msg.rejectbypromiseid() != 0) {
        is_rejected_ = true;
        UpdateOtherProposalId(paxos_msg.rejectbypromiseid());
    }
}

void Proposer::OnPrepareTimeout() {
    if (GetInstanceId() != timeout_instance_id_) {
        return;
    }
    Prepare();
}

void Proposer::Propose() {
    // set the value
    
    // check whether it can skip prepare without rejection
    // else goes into Prepare()
    if (can_skip_prepare_) {
        Accept();
    }
    else {
        Prepare();
    }
}

void Proposer::UpdateOtherProposalId(const uint64_t other_proposal_id) {
    if (other_proposal_id > highest_proposal_id_by_others_) {
        highest_proposal_id_by_others_ = other_proposal_id;
    }
}

}
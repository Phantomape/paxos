#include "instance.h"
#include "proposer.h"
#include "learner.h"
#include <iostream>
#include <string>

namespace paxos {

Proposer::Proposer(
        const Config * config,
        const Communicate * communicate,
        const Instance * poInstance,
        const Learner * poLearner,
        const IoLoop * poIOLoop)
    : Base(config, communicate, poInstance), msg_counter(config) {
    config_ = (Config*)config;
    learner_ = (Learner*)poLearner;
    ioloop_ = (IoLoop *)poIOLoop;
    is_preparing_ = false;
    is_accepting_ = false;
    can_skip_prepare_ = false;

    InitInstance();

    prepare_timer_id_ = 0;
    accept_timer_id_ = 0;
    timeout_instance_id_ = 0;

    last_prepare_timeout_ms_ = config_->GetPrepareTimeoutMs();
    last_accept_timeout_ms_ = config_->GetAcceptTimeoutMs();

    is_rejected_ = false;
}

Proposer::~Proposer() {
}

/**
 * @brief Function that sends accept request to acceptors
 * 
 */
void Proposer::Accept() {
    // Logging

    // Update necessary states here
    ExitPrepare();
    is_accepting_ = true;

    PaxosMsg paxos_msg;
    paxos_msg.set_msgtype(MsgType_PaxosAccept);
    paxos_msg.set_instanceid(GetInstanceId());
    paxos_msg.set_proposalid(GetProposalId());
    paxos_msg.set_value(GetValue());
    paxos_msg.set_lastchecksum(GetLastChecksum());

    msg_counter.Init();

    // Why is this needed
    AddAcceptTimer();

    // Send accept request message to acceptors
    BroadcastMessage(paxos_msg, BroadcastMessage_Type_RunSelf_Final);
}

void Proposer::AddPreAcceptValue(
        const Ballot & other_pre_accept_ballot,
        const std::string & other_pre_accept_val
    ) {
    if (other_pre_accept_ballot.isnull()) {
        return;
    }

    if (other_pre_accept_ballot > highest_other_pre_accept_ballot_) {
        highest_other_pre_accept_ballot_ = other_pre_accept_ballot;
        val_ = other_pre_accept_val;
    }
}

void Proposer::AddPrepareTimer(const int timeout_ms) {
    if (prepare_timer_id_ > 0) {
        ioloop_->RemoveTimer(prepare_timer_id_);
    }

    if (timeout_ms > 0) {
        ioloop_->AddTimer(
                timeout_ms,
                Timer_Proposer_Prepare_Timeout,
                prepare_timer_id_);
        return;
    }

    ioloop_->AddTimer(
            last_prepare_timeout_ms_,
            Timer_Proposer_Prepare_Timeout,
            prepare_timer_id_);

    timeout_instance_id_ = GetInstanceId();

    last_prepare_timeout_ms_ *= 2;
    if (last_prepare_timeout_ms_ > MAX_PREPARE_TIMEOUTMS) {
        last_prepare_timeout_ms_ = MAX_PREPARE_TIMEOUTMS;
    }
}

/**
 * @brief Function that sets timeout when proposer receives a rejected message
 * from acceptors
 *
 * @param timeout_ms
 */
void Proposer::AddAcceptTimer(const int timeout_ms) {
    if (accept_timer_id_ > 0) {
        ioloop_->RemoveTimer(accept_timer_id_);
    }

    if (timeout_ms > 0) {
        ioloop_->AddTimer(
                timeout_ms,
                Timer_Proposer_Accept_Timeout,
                accept_timer_id_);
        return;
    }

    ioloop_->AddTimer(
            last_accept_timeout_ms_,
            Timer_Proposer_Accept_Timeout,
            accept_timer_id_);

    timeout_instance_id_ = GetInstanceId();

    last_accept_timeout_ms_ *= 2;
    if (last_accept_timeout_ms_ > MAX_ACCEPT_TIMEOUTMS) {
        last_accept_timeout_ms_ = MAX_ACCEPT_TIMEOUTMS;
    }
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

void Proposer::Prepare(const bool need_new_ballot) {
    std::cout << "Proposer::Prepare()" << std::endl;

    time_stat_.Point();

    ExitAccept();
    is_preparing_ = true;
    can_skip_prepare_ = false;
    rejected_ = false;

    highest_other_pre_accept_ballot_.reset();
    if (need_new_ballot) {
        // Init new proposer state
        proposal_id_ = std::max(proposal_id_, highest_proposal_id_by_others_) + 1;
    }

    // Calculate votes
    PaxosMsg send_paxos_msg;
    send_paxos_msg.set_msgtype(MsgType_PaxosPrepare);
    send_paxos_msg.set_instanceid(GetInstanceId());
    send_paxos_msg.set_proposalid(GetProposalId());
    send_paxos_msg.set_nodeid(config_->GetMyNodeID());

    msg_counter.Init();

    AddPrepareTimer();

    BroadcastMessage(send_paxos_msg);
}

/**
 * @brief Function entry after receiving the response message from acceptors, this
 * function will be renamed to OnAcceptResponse() later and it is called in insta-
 * nce.cc
 *
 * @param paxos_msg
 */
void Proposer::OnAcceptReply(const PaxosMsg &paxos_msg) {
    // Log is required here

    // The original comment in the code says "Skip the message when the proposer 
    // is not proposing", which is really confusing by checking a state called
    // is_accepting_ to verify whether the proposer is proposing
    if (!is_accepting_) {
        return;
    }

    // Skip the message if proposal id is not the same, I still have no idea
    // when such thing would happend. Could be combined with the code above by 
    // adding detailed description
    if (paxos_msg.proposalid() != proposal_id_) {
        return;
    }

    msg_counter.AddReceivedMsg(paxos_msg.nodeid());

    // Still wondering whether the two large if{..} blocks can be merged together.
    // Will try to do so after adding jepsen test and unit test.
    if (paxos_msg.rejectbypromiseid() == 0) {
        // The response is a reolved message
        msg_counter.AddAcceptedMsg(paxos_msg.nodeid());
    }
    else {
        // The response is a rejected message
        msg_counter.AddReceivedMsg(paxos_msg.nodeid());
        is_rejected_ = true;
        UpdateOtherProposalId(paxos_msg.rejectbypromiseid());
    }

    // It seems to be undefined behavior by writing if{...}else if{...} block, 
    // should take a look during refactor
    if (msg_counter.IsPassed()) {
        can_skip_prepare_ = true;
        ExitAccept();
        learner_->ProposerSendSuccess(GetInstanceId(), GetProposalId());
    }
    else if (msg_counter.IsRejected() || msg_counter.IsAllReceived()) {
        // If the response message is a rejected message, it should restart the
        // prepare stage after 30ms
        AddAcceptTimer(Util::FastRand() % 30 + 10);
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
    Prepare(is_rejected_);
}

void Proposer::OnExpiredAcceptReply(const PaxosMsg & paxos_msg) {
    if (paxos_msg.rejectbypromiseid() != 0) {
        is_rejected_ = true;
        SetOtherProposalId(paxos_msg.rejectbypromiseid());
    }
}

void Proposer :: OnExpiredPrepareReply(const PaxosMsg & paxos_msg) {
    if (paxos_msg.rejectbypromiseid() != 0) {
        is_rejected_ = true;
        SetOtherProposalId(paxos_msg.rejectbypromiseid());
    }
}

void Proposer::OnPrepareReply(const PaxosMsg &recv_paxos_msg) {
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

    if (recv_paxos_msg.rejectbypromiseid() == 0) {
        Ballot ballot(recv_paxos_msg.preacceptid(), recv_paxos_msg.preacceptnodeid());
        msg_counter.AddAcceptedMsg(recv_paxos_msg.nodeid());
        AddPreAcceptValue(ballot, recv_paxos_msg.value());
    }
    else {
        is_rejected_ = true;
        msg_counter.AddRejectedMsg(recv_paxos_msg.nodeid());
        SetOtherProposalId(recv_paxos_msg.rejectbypromiseid());
    }

    if (msg_counter.IsPassed() || msg_counter.IsAllReceived()) {
        int used_time_ms = time_stat_.Point();
        can_skip_prepare_ = true;

        Accept();
    }
    else if (msg_counter.IsRejected()) {
        AddPrepareTimer(Util::FastRand() % 30 + 10); // wtf is this
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
    Prepare(is_rejected_);
}

void Proposer::Propose(const std::string& val) {
    std::cout << "Proposer::Propose()" << std::endl;

    if (val_.size() == 0) {
        val_ = val;
    }

    last_prepare_timeout_ms_ = START_PREPARE_TIMEOUTMS;
    last_accept_timeout_ms_ = START_ACCEPT_TIMEOUTMS;

    // check whether it can skip prepare without rejection
    // else goes into Prepare()
    if (can_skip_prepare_) {
        Accept();
    }
    else {
        Prepare(is_rejected_);
    }
}

void Proposer::SetOtherProposalId(const uint64_t other_proposal_id) {
    if (other_proposal_id > highest_proposal_id_by_others_) {
        highest_proposal_id_by_others_ = other_proposal_id;
    }
}

void Proposer::UpdateOtherProposalId(const uint64_t other_proposal_id) {
    if (other_proposal_id > highest_proposal_id_by_others_) {
        highest_proposal_id_by_others_ = other_proposal_id;
    }
}

}

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

const uint32_t Acceptor::GetChecksum() const {
    return checksum_;
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

/**
 * @brief Function that defines acceptor's behaviors on receiving a prepare 
 * request from proposers, will be renamed to OnPrepareRequest() in refactor
 *
 * @param recv_paxos_msg
 * @return int
 */
int Acceptor::OnPrepare(const PaxosMsg &recv_paxos_msg) {
    // Logging

    PaxosMsg prepare_response_msg;
    prepare_response_msg.set_instanceid(GetInstanceId());
    prepare_response_msg.set_nodeid(config_->GetMyNodeID());
    prepare_response_msg.set_proposalid(recv_paxos_msg.proposalid());
    prepare_response_msg.set_msgtype(MsgType_PaxosPrepareReply);

    Ballot ballot(recv_paxos_msg.proposalid(), recv_paxos_msg.nodeid());
    if (ballot >= GetPromiseBallot()) {
        prepare_response_msg.set_preacceptid(GetAcceptedBallot().proposal_id_);
        prepare_response_msg.set_preacceptnodeid(GetAcceptedBallot().node_id_);

        if (GetAcceptedBallot().proposal_id_ > 0) {
            prepare_response_msg.set_value(GetAcceptedValue());
        }
        SetPromiseBallot(ballot);

        // Persist value
        int res = Persist(GetInstanceId(), GetLastChecksum());
        if (res != 0) {
            return -1;
        }
    }
    else {
        prepare_response_msg.set_rejectbypromiseid(GetPromiseBallot().proposal_id_);
    }

    uint64_t send_node_id = recv_paxos_msg.nodeid();
    SendMessage(send_node_id, prepare_response_msg);

    return 0;
}

/**
 * @brief Function that persist data into disk
 *
 * @param instance_id
 * @param last_checksum
 * @return int
 */
int Acceptor::Persist(const uint64_t instance_id, const uint32_t last_checksum) {
    // Sth. mysterious
    if (instance_id > 0 && last_checksum == 0) {
        checksum_ = 0;
    } else if (accepted_val_.size() > 0) {
        checksum_ = crc32(
            last_checksum, 
            (const uint8_t*)accepted_val_.data(), 
            accepted_val_.size(), 
            CRC32SKIP
        );
    }

    // Even more mysterious operations
    AcceptorStateData state;
    state.set_instanceid(instance_id);
    state.set_promiseid(promised_ballot_.proposal_id_);
    state.set_promisenodeid(promised_ballot_.node_id_);
    state.set_acceptedid(accepted_ballot_.proposal_id_);
    state.set_acceptednodeid(accepted_ballot_.node_id_);
    state.set_acceptedvalue(accepted_val_);
    state.set_checksum(checksum_);

    WriteOptions write_options;
    write_options.sync = config_->LogSync();
    if (write_options.sync){
        sync_times_++;
        if (sync_times_ > config_->SyncInterval()) {
            sync_times_ = 0;
        } else {
            write_options.sync = false;
        }
    }

    // Damn, I hate the author's style
    int ret = paxos_log_.WriteState(write_options, config_->GetMyGroupIdx(), instance_id, state);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

/**
 * @brief Function that defines acceptor's behaviors on receiving an accept
 * request message
 *
 * @param paxos_msg
 */
void Acceptor::OnAccept(const PaxosMsg &paxos_msg) {
    // Logging

    PaxosMsg accept_response_msg;
    accept_response_msg.set_instanceid(GetInstanceId());
    accept_response_msg.set_nodeid(config_->GetMyNodeID());
    accept_response_msg.set_proposalid(paxos_msg.proposalid());
    accept_response_msg.set_msgtype(MsgType_PaxosAcceptReply);

    Ballot ballot(paxos_msg.proposalid(), paxos_msg.nodeid());
    if (ballot >= GetPromiseBallot()) {
        // ???
        SetPromiseBallot(ballot);
        SetAcceptedBallot(ballot);
        SetAcceptedValue(paxos_msg.value());

        int ret = Persist(GetInstanceId(), GetLastChecksum());
        if (ret != 0) {
            return;
        }
    } else {
        accept_response_msg.set_rejectbypromiseid(GetPromiseBallot().proposal_id_);
    }

    uint64_t iReplyNodeID = paxos_msg.nodeid();
    SendMessage(iReplyNodeID, accept_response_msg);
}

}

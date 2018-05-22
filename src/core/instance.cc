#include "def.h"
#include "instance.h"
#include "acceptor.h"
#include "learner.h"
#include "proposer.h"

namespace paxos {

Instance::Instance(
        const Config * poConfig, 
        const LogStorage * poLogStorage,
        const Communicate * poMsgTransport,
        const Options & oOptions)
    : state_machine_fac_(poConfig->GetMyGroupIdx()),
    ioloop_((Config *)poConfig, this),
    acceptor_(poConfig, poMsgTransport, this, poLogStorage), 
    learner_(poConfig, poMsgTransport, this, &acceptor_, poLogStorage, &ioloop_, &checkpoint_mgr_, &state_machine_fac_),
    proposer_(poConfig, poMsgTransport, this, &learner_, &ioloop_),
    paxos_log_(poLogStorage),
    commit_ctx_((Config *)poConfig),
    committer_((Config *)poConfig, &commit_ctx_, &ioloop_, &state_machine_fac_),
    checkpoint_mgr_((Config *)poConfig, &state_machine_fac_, (LogStorage *)poLogStorage, oOptions.use_checkpoint_replayer_),
    m_oOptions(oOptions), m_bStarted(false) {
    config_ = (Config *)poConfig;
    communicate_ = (Communicate *)poMsgTransport;
    commit_timer_id_ = 0;
    last_checksum_= 0;
}

Instance::~Instance() {}

void Instance::CheckNewValue() {

}

int Instance::ForwardToAcceptor(const PaxosMsg& paxos_msg, const bool is_retry) {
    std::cout << "Instance::ForwardToAcceptor()" << std::endl;
    // Sth. related to IsImFollower

    if (paxos_msg.instanceid() != acceptor_.GetInstanceId()) {
    
    }

    if (paxos_msg.instanceid() == acceptor_.GetInstanceId() + 1) {
        PaxosMsg send_paxos_msg = paxos_msg;
        send_paxos_msg.set_instanceid(acceptor_.GetInstanceId());
        send_paxos_msg.set_msgtype(1);

        ForwardToLearner(send_paxos_msg);
    }

    if (paxos_msg.instanceid() == acceptor_.GetInstanceId()) {
        if (paxos_msg.msgtype() == 1) {
            return acceptor_.OnPrepare(paxos_msg);
        }
        else if (paxos_msg.msgtype() == 2) {
            acceptor_.OnAccept(paxos_msg);
        }
    }
    else if (is_retry == false && paxos_msg.instanceid() > acceptor_.GetInstanceId()) {
        if (paxos_msg.instanceid() >= learner_.GetLatestInstanceID()) {
            if (paxos_msg.instanceid() < acceptor_.GetInstanceId() + RETRY_QUEUE_MAX_LEN) {
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

    if (paxos_msg.instanceid() != proposer_.GetInstanceId()) {
        if (paxos_msg.instanceid() + 1 == proposer_.GetInstanceId()) {
            if (paxos_msg.msgtype() == 1) {
                proposer_.OnPrepareRejected(paxos_msg);
            }
            else if (paxos_msg.msgtype() == 2) {
                proposer_.OnAcceptRejected(paxos_msg);
            }
        }

        return 0;
    }

    if (paxos_msg.msgtype() == 3) {
        proposer_.OnPrepare(paxos_msg);
    }
    else if (paxos_msg.msgtype() == 4) {
        proposer_.OnAccept(paxos_msg);
    }

    return 0;
}

int Instance :: GetInstanceValue(const uint64_t llInstanceID, std::string & sValue, int & iSMID)
{
    iSMID = 0;

    if (llInstanceID >= acceptor_.GetInstanceId())
    {
        return Paxos_GetInstanceValue_Value_Not_Chosen_Yet;
    }

    AcceptorStateData oState; 
    /*
    int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), llInstanceID, oState);
    if (ret != 0 && ret != 1)
    {
        return -1;
    }

    if (ret == 1)
    {
        return Paxos_GetInstanceValue_Value_NotExist;
    }
    */

    memcpy(&iSMID, oState.acceptedvalue().data(), sizeof(int));
    sValue = std::string(oState.acceptedvalue().data() + sizeof(int), oState.acceptedvalue().size() - sizeof(int));

    return 0;
}

int Instance::Init() {
    std::cout << "Instance::Init()" << std::endl;
    int res = acceptor_.Init();
    if (res != 0) {
        return res;
    }

    // Sth. related to check point
    uint64_t CurrentInstanceId = 0;
    if (CurrentInstanceId < acceptor_.GetInstanceId()) {
        CurrentInstanceId = acceptor_.GetInstanceId();
    }
    else {
        if (CurrentInstanceId > acceptor_.GetInstanceId()) {
            // Do sth.
            acceptor_.InitInstance();
        }

        acceptor_.SetInstanceId(CurrentInstanceId);
    }

    learner_.SetInstanceId(CurrentInstanceId);
    proposer_.SetInstanceId(CurrentInstanceId);

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
    learner_.InitLearnerSynchronizer();
    // Start ioloop ycheckpoint
    is_started_ = true;
}

void Instance::Stop() {
    std::cout << "Instance::Stop()" << std::endl;
    if (is_started_) {
        // Stop ioloop and checkpoint
        learner_.Stop();
    }
}

}

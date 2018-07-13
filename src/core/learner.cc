#include "acceptor.h"
#include "state_machine_base.h"
#include "checkpoint_mgr.h"
#include "learner.h"

namespace paxos {

/**
 * @brief Construct a new Learner:: Learner object
 * 
 * @param config 
 * @param communicate 
 * @param instance 
 * @param acceptor 
 * @param log_storage 
 * @param ioloop 
 * @param checkpoint_mgr 
 * @param state_machine_fac 
 */
Learner::Learner(
        const Config* config, 
        const Communicate* communicate,
        const Instance* instance,
        const Acceptor* acceptor,
        const LogStorage* log_storage,
        const IoLoop * ioloop,
        const CheckpointMgr * checkpoint_mgr,
        const StateMachineFac * state_machine_fac)
        : Base(config, communicate, instance), 
        paxos_log_(log_storage), 
        learner_synchronizer_((Config *)config, this, &paxos_log_) {
    acceptor_ = (Acceptor *)acceptor;
    InitInstance();

    ioloop_ = (IoLoop *)ioloop;

    checkpoint_mgr = (CheckpointMgr *)checkpoint_mgr;
    state_machine_fac_ = (StateMachineFac *)state_machine_fac;
    //m_poCheckpointSender = nullptr;

    //m_llHighestSeenInstanceID = 0;
    //m_iHighestSeenInstanceID_FromNodeID = nullnode;

    //m_bIsIMLearning = false;

    //m_llLastAckInstanceID = 0;
}

Learner::~Learner() {
}

const uint64_t Learner::GetLatestInstanceID() {
    return highest_instance_id_;
}

void Learner::InitLearnerSynchronizer() {
    learner_synchronizer_.Start();
}

/**
 * @brief Function that sends sync signal to other learners
 *
 * @param instance_id
 * @param proposal_id
 */
void Learner::ProposerSendSuccess(
        const uint64_t instance_id,
        const uint64_t proposal_id
    ) {
    PaxosMsg paxos_msg;
    paxos_msg.set_msgtype(MsgType_PaxosLearner_ProposerSendSuccess);
    paxos_msg.set_instanceid(instance_id);
    paxos_msg.set_nodeid(config_->GetMyNodeID());
    paxos_msg.set_proposalid(proposal_id);
    paxos_msg.set_lastchecksum(GetLastChecksum());

    BroadcastMessage(paxos_msg, BroadcastMessage_Type_RunSelf_First);
}

/**
 * @brief Function that describe what learner should do when the message sent by
 *        proposer is chosen.
 *
 * @param paxos_msg
 */
void Learner::OnProposerSendSuccess(const PaxosMsg& paxos_msg) {
    // Maybe put a log here

    // In normal situation, all nodes will participate in the paxos process(maybe
    // sth. more clear), so all nodes will have the same instance id. If instance
    // id not the same, ignore this message
    if (paxos_msg.instanceid() != GetInstanceId()) {
        return;
    }

    // There is sth. I don't understand here in the original code. LOL.

    // Record learned value
    LearnValueWithoutWrite(
        paxos_msg.instanceid(),
        acceptor_->GetAcceptedValue(),
        acceptor_->GetChecksum()
    );

    // This function will sync the learner with its followers
    TransmitToFollower();
}

/**
 * @brief Function that sync the learned value to the followers
 * 
 */
void Learner::TransmitToFollower() {

}

void Learner::Stop() {
    std::cout << "Learner::Stop()" << std::endl;
    learner_synchronizer_.Stop();
    // Stop checkpoint if needed
}

void Learner::InitInstance() {
    learned_val_ = "";
    is_learned_ = false;
    new_checksum_ = 0;
}

}

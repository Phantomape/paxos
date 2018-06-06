#include "acceptor.h"
#include "state_machine_base.h"
#include "checkpoint_mgr.h"
#include "learner.h"

namespace paxos {

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
    paxos_log_(log_storage), learner_synchronizer_((Config *)config, this, &paxos_log_) {
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

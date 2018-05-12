#include "acceptor.h"
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
    paxos_log_(log_storage), learner_synchronizer_((Config *)config, this, &paxos_log_),
    m_oCheckpointReceiver((Config *)poConfig, (LogStorage *)poLogStorage)
{
    acceptor_ = (Acceptor *)acceptor;
    InitInstance();

    m_iAskforlearn_noopTimerID = 0;
    m_poIOLoop = (IOLoop *)poIOLoop;

    checkpoint_mgr_ = (CheckpointMgr *)checkpoint_mgr;
    state_machine_fac_ = (StateMachineFac *)state_machine_fac;
    //m_poCheckpointSender = nullptr;

    m_llHighestSeenInstanceID = 0;
    m_iHighestSeenInstanceID_FromNodeID = nullnode;

    m_bIsIMLearning = false;

    m_llLastAckInstanceID = 0;
}

Learner::Learner(const Instance* instance, const Acceptor* acceptor) : Base(instance) {
    this->acceptor = (Acceptor*)acceptor;

    highest_instance_id_ = 0;
}

Learner::~Learner() {

}

const uint64_t Learner::GetLatestInstanceID() {
    return highest_instance_id_;
}

void Learner::InitLearnerSynchronizer() {
    learner_synchronizer.Start();
}

void Learner::Stop() {
    std::cout << "Learner::Stop()" << std::endl;
    learner_synchronizer.Stop();
    // Stop checkpoint if needed
}

}
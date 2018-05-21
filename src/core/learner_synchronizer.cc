#include "internal_options.h"
#include "learner_synchronizer.h"
#include "learner.h"
#include <iostream>

namespace paxos {

LearnerSynchronizer::LearnerSynchronizer() {
    std::cout << "LearnerSynchronizer::LearnerSynchronizer()" << std::endl;
}

<<<<<<< 66d582b17176337b4f44afcd598e3a02b447289c
LearnerSynchronizer::LearnerSynchronizer(Config* config, Learner* learner, PaxosLog* paxos_log)
    : config_(config), learner_(learner), paxos_log_(paxos_log)
{
    ack_lead_ = LEARNER_SYNCHRONIZER_ACK_LEAD; 
    is_ended_ = false;
    is_started_ = false;
=======
LearnerSynchronizer::LearnerSynchronizer(Config * poConfig, Learner * poLearner, PaxosLog * poPaxosLog)
    : m_poConfig(poConfig), m_poLearner(poLearner), m_poPaxosLog(poPaxosLog)
{
    m_iAckLead = LEARNER_SYNCHRONIZER_ACK_LEAD; 
    m_bIsEnd = false;
    m_bIsStart = false;
>>>>>>> updated some interfaces
}

LearnerSynchronizer::~LearnerSynchronizer() {
    std::cout << "LearnerSynchronizer::~LearnerSynchronizer()" << std::endl;
}

void LearnerSynchronizer::Ack() {
    std::cout << "LearnerSynchronizer::Ack()" << std::endl;
}

void LearnerSynchronizer::Confirm() {
    std::cout << "LearnerSynchronizer::Confirm()" << std::endl;
}

void LearnerSynchronizer::Prepare() {
    std::cout << "LearnerSynchronizer::Prepare()" << std::endl;
}

void LearnerSynchronizer::Run() {
    std::cout << "LearnerSynchronizer::Run()" << std::endl;
}

void LearnerSynchronizer::Stop() {
    std::cout << "LearnerSynchronizer::Stop()" << std::endl;
}

}
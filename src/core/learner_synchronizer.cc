#include "learner_synchronizer.h"
#include "learner.h"
#include <iostream>

namespace paxos {

LearnerSynchronizer::LearnerSynchronizer(Config* config, Learner* learner, PaxosLog* paxos_log)
    : config_(config), learner_(learner), paxos_log_(paxos_log)
{
    ack_lead_ = LEARNER_SYNCHRONIZER_ACK_LEAD;
    is_ended_ = false;
    is_started_ = false;
}

LearnerSynchronizer::~LearnerSynchronizer() {
}

void LearnerSynchronizer::Ack() {
}

void LearnerSynchronizer::Confirm() {
}

void LearnerSynchronizer::Prepare() {
}

void LearnerSynchronizer::Run() {
}

void LearnerSynchronizer::Stop() {
}

}
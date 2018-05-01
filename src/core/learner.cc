#include "learner.h"

namespace paxos {

Learner::Learner() {
}

Learner::~Learner() {

}

void Learner::InitLearnerSynchronizer() {
    learner_synchronizer.Start();
}

}
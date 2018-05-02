#include "acceptor.h"
#include "learner.h"

namespace paxos {

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
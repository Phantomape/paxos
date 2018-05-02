#include "acceptor.h"
#include "learner.h"

namespace paxos {

Learner::Learner(const Instance* instance, const Acceptor* acceptor) : Base(instance) {
    this->acceptor = (Acceptor*)acceptor;
}

Learner::~Learner() {

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
#include "learner_synchronizer.h"
#include <iostream>

namespace paxos {

LearnerSynchronizer::LearnerSynchronizer() {
    std::cout << "LearnerSynchronizer::LearnerSynchronizer()" << std::endl;
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
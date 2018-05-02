#pragma once

#include "base.h"
#include "learner_synchronizer.h"

namespace paxos {

class Acceptor;

class Learner : public Base {
public:
    Learner(const Instance* instance, const Acceptor* acceptor);
    ~Learner();

    const uint64_t GetLatestInstanceID();

    virtual void InitInstance();

    const bool IsLearned();

    void InitLearnerSynchronizer();

    void Stop();
private:
    Acceptor* acceptor;
    LearnerSynchronizer learner_synchronizer;

    uint64_t highest_instance_id_;
};

}
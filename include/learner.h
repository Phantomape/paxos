#pragma once

#include "acceptor.h"
#include "base.h"
#include "learner_synchronizer.h"

namespace paxos {

class Learner : public Base {
public:
    Learner();
    ~Learner();

    virtual void InitInstance();

    const bool IsLearned();

    void InitLearnerSynchronizer();

    void Stop();
private:
    Acceptor* acceptor;
    LearnerSynchronizer learner_synchronizer;
};

}
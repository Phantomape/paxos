#pragma once

#include "base.h"

namespace paxos {

class Learner : public Base {
public:
    Learner();
    ~Learner();

    virtual void InitInstance();

    const bool IsLearned();
};

}
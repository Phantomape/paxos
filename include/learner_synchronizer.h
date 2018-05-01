#pragma once

#include "concurrent.h"

namespace paxos {

class LearnerSynchronizer : public Thread{
public:
    LearnerSynchronizer();
    ~LearnerSynchronizer();

    void Ack();
    void Confirm();
    void Prepare();
    void Run();
    void Stop();
};

}
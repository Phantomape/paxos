#include "instance.h"

namespace paxos {

Instance::Instance() {}

Instance::~Instance() {}

int Instance::Init() {
    std::cout << "Instance::Init()" << std::endl;
    int res = acceptor.Init();
    if (res != 0) {
        return res;
    }

    // Sth. related to check point
    uint64_t CurrentInstanceId = 0;
    if (CurrentInstanceId < acceptor.GetInstanceId()) {
        CurrentInstanceId = acceptor.GetInstanceId();
    }
    else {
        if (CurrentInstanceId > acceptor.GetInstanceId()) {
            // Do sth.
            acceptor.InitInstance();
        }

        acceptor.SetInstanceId(CurrentInstanceId);
    }

    learner.SetInstanceId(CurrentInstanceId);
    proposer.SetInstanceId(CurrentInstanceId);

    // Sth. I don't understand
    return 0;
}

void Instance::Start() {
    std::cout << "Instance::Start()" << std::endl;
    learner.InitLearnerSynchronizer();
    // Start ioloop ycheckpoint
    is_started_ = true;
}

void Instance::Stop() {
    std::cout << "Instance::Stop()" << std::endl;
    if (is_started_) {
        // Stop ioloop and checkpoint
        learner.Stop();
    }
}

}

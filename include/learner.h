#pragma once

#include "base.h"
#include "ioloop.h"
#include "learner_synchronizer.h"
#include "paxos_log.h"

namespace paxos {

class Acceptor;
class CheckpointMgr;
class StateMachineFac;

class Learner : public Base {
public:
    Learner(
            const Config* config, 
            const Communicate* communicate,
            const Instance* instance,
            const Acceptor* acceptor,
            const LogStorage* log_storage,
            const IoLoop* ioloop,
            const CheckpointMgr* checkpoint_mgr,
            const StateMachineFac * state_machine_fac);

    Learner(const Instance* instance, const Acceptor* acceptor);
    
    ~Learner();

    const uint64_t GetLatestInstanceID();

    virtual void InitInstance();

    const bool IsLearned();

    void InitLearnerSynchronizer();

    void Stop();
private:
    Acceptor* acceptor_;
    LearnerSynchronizer learner_synchronizer_;

    uint64_t highest_instance_id_;

    std::string learned_val_;
    bool is_learned_;
    uint32_t new_checksum_;

    CheckpointMgr* checkpoint_mgr_;
    StateMachineFac* state_machine_fac_;

    Config* config_;
    PaxosLog paxos_log_;
};

}
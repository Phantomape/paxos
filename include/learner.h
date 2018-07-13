#pragma once

#include "base.h"
#include "checkpoint_mgr.h"
#include "ioloop.h"
#include "learner_synchronizer.h"
#include "paxos_log.h"

namespace paxos {

class Acceptor;
class CheckpoingMgr;
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
            const StateMachineFac* state_machine_fac
            );

    ~Learner();

    const uint64_t GetLatestInstanceID();

    virtual void InitInstance();

    const bool IsLearned();

    void InitLearnerSynchronizer();

    void LearnValueWithoutWrite(
        const uint64_t instance_id,
        const std::string & val,
        const uint32_t checksum
    );

    void TransmitToFollower();

    void ProposerSendSuccess(
        const uint64_t instance_id,
        const uint64_t proposal_id
    );

    void OnProposerSendSuccess(const PaxosMsg& paxos_msg);

    void Stop();

private:
    Acceptor* acceptor_;
    LearnerSynchronizer learner_synchronizer_;

    uint64_t highest_instance_id_;

    IoLoop* ioloop_;

    std::string learned_val_;
    bool is_learned_;
    uint32_t new_checksum_;

    Config* config_;
    PaxosLog paxos_log_;

    uint32_t ask_for_learn_noop_timer_id_;

    uint64_t highest_seen_instance_id_;
    uint64_t highest_seen_instance_id_from_node_id_;

    bool is_im_learning_;
    uint64_t last_ack_instance_id_;

    CheckpointMgr * checkpoint_mgr_;
    StateMachineFac * state_machine_fac_;
};

}
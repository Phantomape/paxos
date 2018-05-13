#pragma once

#include "concurrent.h"
#include "internal_options.h"

namespace paxos {

class Learner;

class LearnerSynchronizer : public Thread{
public:
    LearnerSender(Config* config, Learner* learner, PaxosLog* paxos_log);
    ~LearnerSynchronizer();

    void Ack();
    void Confirm();
    void Prepare();
    void Run();
    void Stop();
private:
    Config* config_;
    Learner* learner_;
    PaxosLog* paxos_log_;
    SerialLock lock_;

    bool is_im_sending_;
    uint64_t abs_last_send_time_;

    uint64_t begin_instance_id_;
    uint64_t send_to_node_id_;

    bool is_confirmed_;

    uint64_t ack_instance_id_;
    uint64_t abs_last_ack_time_;
    int ack_lead_;

    bool is_ended_;
    bool is_started_;
};

}
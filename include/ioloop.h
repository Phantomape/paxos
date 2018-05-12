#pragma once

#include "concurrent.h"
#include "instance.h"
#include <queue>

namespace paxos {

#define RETRY_QUEUE_MAX_LEN 300

class Instance;

class IoLoop : public Thread {
public:
    IoLoop(Config * poConfig, Instance * poInstance);

    virtual ~IoLoop();

    void AddNotify();
    int AddRetryPaxosMsg(const PaxosMsg& paxos_msg);
    void ClearRetryQueue();
    void Init(const int timeout_ms);
    void DealWithRetry();
    void Run();
    void Stop();
private:
    bool is_ended_;
    bool is_started_;

    int queue_size_;

    Config * config_;
    Instance* instance_;
    Queue<std::string*> message_queue_;
    std::queue<PaxosMsg> retry_queue_;
};

}
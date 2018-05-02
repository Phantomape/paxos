#pragma once

#include "concurrent.h"
#include "instance.h"
#include <queue>

namespace paxos {

#define RETRY_QUEUE_MAX_LEN 300

class IoLoop : public Thread {
public:
    IoLoop();
    virtual ~IoLoop();

    void Init(const int timeout_ms);
    void Run();
    void Stop();
private:
    bool is_end_;
    bool is_start_;

    int queue_size_;

    Instance* instance;
    Queue<std::string*> message_queue;
    std::queue<PaxosMsg> retry_queue;
};

}
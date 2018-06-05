#pragma once

#include "concurrent.h"
#include "timer.h"
#include <queue>

namespace paxos {

#define RETRY_QUEUE_MAX_LEN 300

class Instance;

class IoLoop : public Thread {
public:
    IoLoop(Config* config, Instance* instance);

    virtual ~IoLoop();

    void Run();

    void Stop();

    void Loop(const int timeout_ms);

    void DealWithRetry();

    void ClearRetryQueue();

    int AddMessage(const char* msg, const int msg_len);

    int AddRetryPaxosMsg(const PaxosMsg& paxos_msg);

    void AddNotify();

    virtual bool AddTimer(const int timeout, const int type, uint32_t& timer_id);

    virtual void RemoveTimer(uint32_t& timer_id);

    void DealwithTimeout(int& next_timeout);

    void DealwithTimeoutOne(const uint32_t timer_id, const int type);

private:
    bool is_ended_;
    bool is_started_;
    Timer timer_;

    std::map<uint32_t, bool> map_timer_id_exist_;

    Queue<std::string*> message_queue_;
    std::queue<PaxosMsg> retry_queue_;

    int queue_mem_size_;

    Config* config_;
    Instance* instance_;
};

}
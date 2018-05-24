#pragma once

#include "concurrent.h"
#include "timer.h"
#include <queue>

namespace paxos {

#define RETRY_QUEUE_MAX_LEN 300

class Instance;

class IoLoop : public Thread
{
public:
    IoLoop(Config * poConfig, Instance * poInstance);

    virtual ~IoLoop();

    void Run();

    void Stop();

    void Init(const int iTimeoutMs);

    void DealWithRetry();

    void ClearRetryQueue();

public:
    int AddMessage(const char * pcMessage, const int iMessageLen);

    int AddRetryPaxosMsg(const PaxosMsg & oPaxosMsg);

    void AddNotify();

public:
    virtual bool AddTimer(const int iTimeout, const int iType, uint32_t & iTimerID);

    virtual void RemoveTimer(uint32_t & iTimerID);

    void DealwithTimeout(int & iNextTimeout);

    void DealwithTimeoutOne(const uint32_t iTimerID, const int iType);

private:
    bool m_bIsEnd;
    bool m_bIsStart;
    Timer m_oTimer;
    std::map<uint32_t, bool> m_mapTimerIDExist;

    Queue<std::string *> m_oMessageQueue;
    std::queue<PaxosMsg> m_oRetryQueue;

    int m_iQueueMemSize;

    Config * m_poConfig;
    Instance * m_poInstance;
};

}
#pragma once

#include "config.h"
#include "state_machine_base.h"
#include "wait_lock.h"
#include <string>
#include <inttypes.h>

namespace paxos
{

class CommitCtx;
class IoLoop;

class Committer
{
public:
    Committer(Config * poConfig, CommitCtx * poCommitCtx, IoLoop * poIOLoop, StateMachineFac * poStateMachineFac);

    ~Committer();

public:
    int NewValueGetID(const std::string & sValue, uint64_t & llInstanceID);

    int NewValueGetID(const std::string & sValue, uint64_t & llInstanceID, StateMachineCtx * poStateMachineCtx);

    int NewValueGetIDNoRetry(const std::string & sValue, uint64_t & llInstanceID, StateMachineCtx * poStateMachineCtx);

    int NewValue(const std::string & sValue);

public:
    void SetTimeoutMs(const int iTimeoutMs);

    void SetMaxHoldThreads(const int iMaxHoldThreads);

    void SetProposeWaitTimeThresholdMS(const int iWaitTimeThresholdMS);

private:
    void LogStatus();

    Config * m_poConfig;
    CommitCtx * m_poCommitCtx;
    IoLoop * m_poIOLoop;
    StateMachineFac * m_poSMFac;

    WaitLock m_oWaitLock;
    int m_iTimeoutMs;

    uint64_t m_llLastLogTime;
};

}

#pragma once

#include "config.h"
#include "serial_lock.h"
#include "state_machine.h"
#include <string>

namespace paxos {

class StateMachine;

class CommitCtx
{
public:
    CommitCtx(Config * poConfig);

    ~CommitCtx();

    void NewCommit(std::string * psValue, StateMachineCtx * poSMCtx, const int iTimeoutMs);
    
    const bool IsNewCommit() const;

    std::string & GetCommitValue();

    void StartCommit(const uint64_t llInstanceID);

    bool IsMyCommit(const uint64_t llInstanceID, const std::string & sLearnValue, StateMachineCtx *& poSMCtx);

    void SetResult(const int iCommitRet, const uint64_t llInstanceID, const std::string & sLearnValue);

    void SetResultOnlyRet(const int iCommitRet);

    int GetResult(uint64_t & llSuccInstanceID);

    const int GetTimeoutMs() const;

private:
    Config * m_poConfig;

    uint64_t m_llInstanceID;
    int m_iCommitRet;
    bool m_bIsCommitEnd;
    int m_iTimeoutMs;

    std::string * m_psValue;
    StateMachineCtx * m_poSMCtx;
    SerialLock m_oSerialLock;
};
}

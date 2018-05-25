#include "def.h"
#include "committer.h"
#include "commit_ctx.h"
#include "ioloop.h"
#include "util.h"

namespace paxos {

Committer::Committer(Config * poConfig, CommitCtx * poCommitCtx, IoLoop * poIOLoop, StateMachineFac * poSMFac)
    : m_poConfig(poConfig), m_poCommitCtx(poCommitCtx), m_poIOLoop(poIOLoop), m_poSMFac(poSMFac), m_iTimeoutMs(-1)
{
    m_llLastLogTime = Time::GetSteadyClockMS();
}

Committer::~Committer()
{
}

int Committer::NewValue(const std::string & sValue)
{
    uint64_t instance_id = 0;
    return NewValueGetID(sValue, instance_id, nullptr);
}

int Committer::NewValueGetID(const std::string & sValue, uint64_t & instance_id)
{
    return NewValueGetID(sValue, instance_id, nullptr);
}

int Committer::NewValueGetID(const std::string & sValue, uint64_t & instance_id, StateMachineCtx * state_machine_ctx) {
    //BP->GetCommiterBP()->NewValue();

    int iRetryCount = 3;
    int ret = PaxosTryCommitRet_OK;
    while(iRetryCount--) {
        TimeStat oTimeStat;
        oTimeStat.Point();

        ret = NewValueGetIDNoRetry(sValue, instance_id, state_machine_ctx);
        if (ret != PaxosTryCommitRet_Conflict) {
            if (ret == 0) {
                std::cout << "NewValueCommitOK" << std::endl;
            }
            else {
                std::cout << "NewValueCommitFail" << std::endl;
            }
            break;
        }

        if (state_machine_ctx != nullptr && state_machine_ctx->state_machine_id_ == MASTER_V_STATE_MACHINE_ID) {
            //master sm not retry
            break;
        }
    }

    return ret;
}

int Committer::NewValueGetIDNoRetry(
    const std::string & sValue, 
    uint64_t & instance_id, 
    StateMachineCtx * state_machine_ctx
    ) {
    LogStatus();

    int get_lock_time_ms = 0;
    bool has_lock = m_oWaitLock.Lock(m_iTimeoutMs, get_lock_time_ms);
    if (!has_lock) {
        if (get_lock_time_ms > 0) {
            //BP->GetCommiterBP()->NewValueGetLockTimeout();
            //PLGErr("Try get lock, but timeout, lockusetime %dms", get_lock_time_ms);
            return PaxosTryCommitRet_Timeout; 
        }
        else { // wtf ??
            //BP->GetCommiterBP()->NewValueGetLockReject();
            //PLGErr("Try get lock, but too many thread waiting, reject");
            return PaxosTryCommitRet_TooManyThreadWaiting_Reject;
        }
    }

    int left_time_ms = -1;
    if (m_iTimeoutMs > 0) {
        left_time_ms = m_iTimeoutMs > get_lock_time_ms ? m_iTimeoutMs - get_lock_time_ms : 0;
        if (left_time_ms < 200) {
            //PLGErr("Get lock ok, but lockusetime %dms too long, lefttimeout %dms", get_lock_time_ms, left_time_ms);
            m_oWaitLock.UnLock();
            return PaxosTryCommitRet_Timeout;
        }
    }

    // Get lock successfully
    int state_machine_id = state_machine_ctx != nullptr ? state_machine_ctx->state_machine_id_ : 0;
    
    std::string sPackSMIDValue = sValue;
    m_poSMFac->PackPaxosValue(sPackSMIDValue, state_machine_id);

    m_poCommitCtx->NewCommit(&sPackSMIDValue, state_machine_ctx, left_time_ms);
    m_poIOLoop->AddNotify();

    int ret = m_poCommitCtx->GetResult(instance_id);

    m_oWaitLock.UnLock();
    return ret;
}

void Committer::SetTimeoutMs(const int iTimeoutMs) {
    m_iTimeoutMs = iTimeoutMs;
}

void Committer::SetMaxHoldThreads(const int iMaxHoldThreads) {
    m_oWaitLock.SetMaxWaitLogCount(iMaxHoldThreads);
}

void Committer::SetProposeWaitTimeThresholdMS(const int iWaitTimeThresholdMS) {
    m_oWaitLock.SetLockWaitTimeThreshold(iWaitTimeThresholdMS);
}

void Committer::LogStatus() {
    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (llNowTime > m_llLastLogTime && llNowTime - m_llLastLogTime > 1000) {
        m_llLastLogTime = llNowTime;
    }
}
    
}



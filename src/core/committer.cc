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
    uint64_t llInstanceID = 0;
    return NewValueGetID(sValue, llInstanceID, nullptr);
}

int Committer::NewValueGetID(const std::string & sValue, uint64_t & llInstanceID)
{
    return NewValueGetID(sValue, llInstanceID, nullptr);
}

int Committer::NewValueGetID(const std::string & sValue, uint64_t & llInstanceID, StateMachineCtx * poSMCtx)
{
    //BP->GetCommiterBP()->NewValue();

    int iRetryCount = 3;
    int ret = PaxosTryCommitRet_OK;
    while(iRetryCount--)
    {
        TimeStat oTimeStat;
        oTimeStat.Point();

        ret = NewValueGetIDNoRetry(sValue, llInstanceID, poSMCtx);
        if (ret != PaxosTryCommitRet_Conflict)
        {
            if (ret == 0)
            {
                //BP->GetCommiterBP()->NewValueCommitOK(oTimeStat.Point());
            }
            else
            {
                //BP->GetCommiterBP()->NewValueCommitFail();
            }
            break;
        }

        //BP->GetCommiterBP()->NewValueConflict();

        if (poSMCtx != nullptr && poSMCtx->state_machine_id_ == MASTER_V_STATE_MACHINE_ID)
        {
            //master sm not retry
            break;
        }
    }

    return ret;
}

int Committer::NewValueGetIDNoRetry(const std::string & sValue, uint64_t & llInstanceID, StateMachineCtx * poSMCtx)
{
    LogStatus();

    int iLockUseTimeMs = 0;
    bool bHasLock = m_oWaitLock.Lock(m_iTimeoutMs, iLockUseTimeMs);
    if (!bHasLock)
    {
        if (iLockUseTimeMs > 0)
        {
            //BP->GetCommiterBP()->NewValueGetLockTimeout();
            //PLGErr("Try get lock, but timeout, lockusetime %dms", iLockUseTimeMs);
            return PaxosTryCommitRet_Timeout; 
        }
        else
        {
            //BP->GetCommiterBP()->NewValueGetLockReject();
            //PLGErr("Try get lock, but too many thread waiting, reject");
            return PaxosTryCommitRet_TooManyThreadWaiting_Reject;
        }
    }

    int iLeftTimeoutMs = -1;
    if (m_iTimeoutMs > 0)
    {
        iLeftTimeoutMs = m_iTimeoutMs > iLockUseTimeMs ? m_iTimeoutMs - iLockUseTimeMs : 0;
        if (iLeftTimeoutMs < 200)
        {
            //PLGErr("Get lock ok, but lockusetime %dms too long, lefttimeout %dms", iLockUseTimeMs, iLeftTimeoutMs);

            //BP->GetCommiterBP()->NewValueGetLockTimeout();

            m_oWaitLock.UnLock();
            return PaxosTryCommitRet_Timeout;
        }
    }

    //PLGImp("GetLock ok, use time %dms", iLockUseTimeMs);
    
    //BP->GetCommiterBP()->NewValueGetLockOK(iLockUseTimeMs);

    //pack smid to value
    int iSMID = poSMCtx != nullptr ? poSMCtx->state_machine_id_ : 0;
    
    std::string sPackSMIDValue = sValue;
    m_poSMFac->PackPaxosValue(sPackSMIDValue, iSMID);

    m_poCommitCtx->NewCommit(&sPackSMIDValue, poSMCtx, iLeftTimeoutMs);
    m_poIOLoop->AddNotify();

    int ret = m_poCommitCtx->GetResult(llInstanceID);

    m_oWaitLock.UnLock();
    return ret;
}

////////////////////////////////////////////////////

void Committer::SetTimeoutMs(const int iTimeoutMs)
{
    m_iTimeoutMs = iTimeoutMs;
}

void Committer::SetMaxHoldThreads(const int iMaxHoldThreads)
{
    m_oWaitLock.SetMaxWaitLogCount(iMaxHoldThreads);
}

void Committer::SetProposeWaitTimeThresholdMS(const int iWaitTimeThresholdMS)
{
    m_oWaitLock.SetLockWaitTimeThreshold(iWaitTimeThresholdMS);
}

////////////////////////////////////////////////////

void Committer::LogStatus()
{
    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (llNowTime > m_llLastLogTime
            && llNowTime - m_llLastLogTime > 1000)
    {
        m_llLastLogTime = llNowTime;
    }
}
    
}



#pragma once

#include "serial_lock.h"

namespace paxos {

#define WAIT_LOCK_USERTIME_AVG_INTERVAL 250

class WaitLock
{
public:
    WaitLock();

    ~WaitLock();

    bool Lock(const int iTimeoutMs, int & iUseTimeMs);

    void UnLock();

    void SetMaxWaitLogCount(const int iMaxWaitLockCount);

    void SetLockWaitTimeThreshold(const int iLockWaitTimeThresholdMS);

public:
    //stat
    int GetNowHoldThreadCount();

    int GetNowAvgThreadWaitTime();

    int GetNowRejectRate();

private:
    void RefleshRejectRate(const int iUseTimeMs);

    bool CanLock();

private:
    SerialLock m_oSerialLock;
    bool m_bIsLockUsing;

    int m_iWaitLockCount;
    int m_iMaxWaitLockCount;

    int m_iLockUseTimeSum;
    int m_iAvgLockUseTime;
    int m_iLockUseTimeCount;

    int m_iRejectRate;
    int m_iLockWaitTimeThresholdMS;
};

}

#pragma once

#include "concurrent.h"
#include <typeinfo>

namespace paxos {

#define CAN_DELETE_DELTA 1000000 
#define DELETE_SAVE_INTERVAL 100

class Config;
class StateMachineFac;
class LogStorage;
class CheckpointMgr;

class Cleaner : public Thread {
public:
    Cleaner(
            Config * poConfig,
            StateMachineFac * poStateMachineFac, 
            LogStorage * poLogStorage,
            CheckpointMgr * poCheckpointMgr);

    ~Cleaner();

    void Stop();

    void Run();

    void Pause();

    void Continue();

    const bool IsPaused() const;

public:
    void SetHoldPaxosLogCount(const uint64_t llHoldCount);

    int FixMinChosenInstanceID(const uint64_t llOldMinChosenInstanceID);

private:
    bool DeleteOne(const uint64_t llInstanceID);

private:
    Config * m_poConfig;
    StateMachineFac * m_poStateMachineFac;
    LogStorage * m_poLogStorage;
    CheckpointMgr * m_poCheckpointMgr;

    uint64_t m_llLastSave;

    bool m_bCanrun;
    bool m_bIsPaused;

    bool m_bIsEnd;
    bool m_bIsStart;

    uint64_t m_llHoldCount;
};
    
}

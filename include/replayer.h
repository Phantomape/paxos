#pragma once

#include "concurrent.h"
#include "paxos_log.h"

namespace paxos {

class Config;
class StateMachineFac;
class LogStorage;
class CheckpointMgr;
    
class Replayer : public Thread {
public:
    Replayer(
            Config * poConfig,
            StateMachineFac * poSMFac, 
            LogStorage * poLogStorage,
            CheckpointMgr * poCheckpointMgr);

    ~Replayer();

    void Stop();

    void Run();

    void Pause();

    void Continue();

    const bool IsPaused() const;

private:
    bool PlayOne(const uint64_t llInstanceID);

    Config * m_poConfig;
    StateMachineFac * m_poSMFac;
    PaxosLog m_oPaxosLog;
    CheckpointMgr * m_poCheckpointMgr;

    bool m_bCanrun;
    bool m_bIsPaused;
    bool m_bIsEnd;
};

}

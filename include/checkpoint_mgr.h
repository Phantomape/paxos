#pragma once

#include "replayer.h"
#include "cleaner.h"
#include "options.h"
#include <set>

namespace paxos {

class CheckpointMgr
{
public:
    CheckpointMgr(
            Config * poConfig,
            StateMachineFac * poSMFac, 
            LogStorage * poLogStorage,
            const bool bUseCheckpointReplayer);

    ~CheckpointMgr();

    int Init();

    void Start();

    void Stop();

    Replayer * GetReplayer();

    Cleaner * GetCleaner();

public:
    int PrepareForAskforCheckpoint(const uint64_t iSendNodeID);

    const bool InAskforcheckpointMode() const;

    void ExitCheckpointMode();

public:
    const uint64_t GetMinChosenInstanceId() const;

    int SetMinChosenInstanceID(const uint64_t llMinChosenInstanceID);

    void SetMinChosenInstanceIDCache(const uint64_t llMinChosenInstanceID);

    const uint64_t GetCheckpointInstanceID() const;

    const uint64_t GetMaxChosenInstanceID() const;

    void SetMaxChosenInstanceID(const uint64_t llMaxChosenInstanceID);

private:
    Config * m_poConfig;
    LogStorage * m_poLogStorage;
    StateMachineFac * m_poStateMachineFac;

    Replayer m_oReplayer;
    Cleaner m_oCleaner;

    uint64_t m_llMinChosenInstanceID;
    uint64_t m_llMaxChosenInstanceID;

    bool m_bInAskforCheckpointMode;
    std::set<uint64_t> m_setNeedAsk;
    uint64_t m_llLastAskforCheckpointTime;

    bool m_bUseCheckpointReplayer;
};

}

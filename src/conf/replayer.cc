#include "config.h"
#include "replayer.h"
#include "log_storage.h"
#include "state_machine_base.h"
#include "checkpoint_mgr.h"
#include "util.h"

namespace paxos {

Replayer::Replayer(
    Config * poConfig,
    StateMachineFac * poSMFac,
    LogStorage * poLogStorage,
    CheckpointMgr * poCheckpointMgr)
    : m_poConfig(poConfig),
    m_poSMFac(poSMFac),
    m_oPaxosLog(poLogStorage),
    m_poCheckpointMgr(poCheckpointMgr),
    m_bCanrun(false),
    m_bIsPaused(true),
    m_bIsEnd(false) {
}

Replayer::~Replayer() {
}

void Replayer::Stop() {
    m_bIsEnd = true;
    Join();
}

void Replayer::Pause() {
    m_bCanrun = false;
}

void Replayer::Continue() {
    m_bIsPaused = false;
    m_bCanrun = true;
}

const bool Replayer:: IsPaused() const {
    return m_bIsPaused;
}

void Replayer::Run() {
    uint64_t llInstanceID = m_poSMFac->GetCheckpointInstanceId(m_poConfig->GetMyGroupIdx()) + 1;

    while (true) {
        if (m_bIsEnd) {
            return;
        }

        if (!m_bCanrun) {
            //PLGImp("Pausing, sleep");
            m_bIsPaused = true;
            Time::MsSleep(1000);
            continue;
        }

        if (llInstanceID >= m_poCheckpointMgr->GetMaxChosenInstanceID()) {
            //PLGImp("now maxchosen instanceid %lu small than excute instanceid %lu, wait",
                    //m_poCheckpointMgr->GetMaxChosenInstanceID(), llInstanceID);
            Time::MsSleep(1000);
            continue;
        }

        bool bPlayRet = PlayOne(llInstanceID);
        if (bPlayRet) {
            llInstanceID++;
        }
        else {
            Time::MsSleep(500);
        }
    }
}

bool Replayer::PlayOne(const uint64_t llInstanceID) {
    AcceptorStateData oState;
    int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), llInstanceID, oState);
    if (ret != 0) {
        return false;
    }

    bool bExecuteRet = m_poSMFac->ExecuteForCheckpoint(
            m_poConfig->GetMyGroupIdx(), llInstanceID, oState.acceptedvalue());
    if (!bExecuteRet) {
        std::cout << "Called Replayer::PlayOne failed" << std::endl;
    }

    return bExecuteRet;
}

}

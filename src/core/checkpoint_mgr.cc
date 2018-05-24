
#include "checkpoint_mgr.h"
#include "config.h"
#include "state_machine_base.h"
#include "log_storage.h"
#include "util.h"

namespace paxos
{

CheckpointMgr::CheckpointMgr(
        Config * poConfig,
        StateMachineFac * poStateMachineFac, 
        LogStorage * poLogStorage,
        const bool bUseCheckpointReplayer) 
    : m_poConfig(poConfig),
    m_poLogStorage(poLogStorage),
    m_poStateMachineFac(poStateMachineFac),
    m_oReplayer(poConfig, poStateMachineFac, poLogStorage, this),
    m_oCleaner(poConfig, poStateMachineFac, poLogStorage, this),
    m_llMinChosenInstanceID(0),
    m_llMaxChosenInstanceID(0),
    m_bInAskforCheckpointMode(false),
    m_bUseCheckpointReplayer(bUseCheckpointReplayer)
{
    m_llLastAskforCheckpointTime = 0;
}

CheckpointMgr::~CheckpointMgr()
{
}

int CheckpointMgr::Init()
{
    int ret = m_poLogStorage->GetMinChosenInstanceID(m_poConfig->GetMyGroupIdx(), m_llMinChosenInstanceID);
    if (ret != 0)
    {
        return ret;
    }

    ret = m_oCleaner.FixMinChosenInstanceID(m_llMinChosenInstanceID);
    if (ret != 0)
    {
        return ret;
    }
    
    return 0;
}

void CheckpointMgr::Start()
{
    if (m_bUseCheckpointReplayer)
    {
        m_oReplayer.Start();
    }
    m_oCleaner.Start();
}

void CheckpointMgr::Stop()
{
    if (m_bUseCheckpointReplayer)
    {
        m_oReplayer.Stop();
    }
    m_oCleaner.Stop();
}

Replayer * CheckpointMgr::GetReplayer()
{
    return &m_oReplayer;
}

Cleaner * CheckpointMgr::GetCleaner()
{
    return &m_oCleaner;
}

int CheckpointMgr::PrepareForAskforCheckpoint(const uint64_t iSendNodeID)
{
    if (m_setNeedAsk.find(iSendNodeID) == m_setNeedAsk.end())
    {
        m_setNeedAsk.insert(iSendNodeID);
    }

    if (m_llLastAskforCheckpointTime == 0)
    {
        m_llLastAskforCheckpointTime = Time::GetSteadyClockMS();
    }

    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (llNowTime > m_llLastAskforCheckpointTime + 60000)
    {
        //PLGImp("no majority reply, just ask for checkpoint");
    }
    else
    {

        if ((int)m_setNeedAsk.size() < m_poConfig->GetMajorityCount())
        {
            //PLGImp("Need more other tell us need to askforcheckpoint");
            return -2;
        }
    }
    
    m_llLastAskforCheckpointTime = 0;
    m_bInAskforCheckpointMode = true;

    return 0;
}

/////////////////////////////////////////////////////

const bool CheckpointMgr::InAskforcheckpointMode() const
{
    return m_bInAskforCheckpointMode;
}

void CheckpointMgr::ExitCheckpointMode()
{
    m_bInAskforCheckpointMode = false;
}

const uint64_t CheckpointMgr::GetCheckpointInstanceID() const
{
    //return m_poStateMachineFac->GetCheckpointInstanceID(m_poConfig->GetMyGroupIdx());
}

const uint64_t CheckpointMgr::GetMinChosenInstanceId() const
{
    //return m_llMinChosenInstanceID;
}

int CheckpointMgr::SetMinChosenInstanceID(const uint64_t llMinChosenInstanceID)
{ 
    WriteOptions oWriteOptions;
    oWriteOptions.sync = true;

    int ret = m_poLogStorage->SetMinChosenInstanceID(oWriteOptions, m_poConfig->GetMyGroupIdx(), llMinChosenInstanceID);
    if (ret != 0)
    {
        return ret;
    }

    m_llMinChosenInstanceID = llMinChosenInstanceID;

    return 0;
}

void CheckpointMgr::SetMinChosenInstanceIDCache(const uint64_t llMinChosenInstanceID)
{
    m_llMinChosenInstanceID = llMinChosenInstanceID;
}

void CheckpointMgr::SetMaxChosenInstanceID(const uint64_t llMaxChosenInstanceID)
{
    m_llMaxChosenInstanceID = llMaxChosenInstanceID;
}

const uint64_t CheckpointMgr::GetMaxChosenInstanceID() const
{
    return m_llMaxChosenInstanceID;
}

}
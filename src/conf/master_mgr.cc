#include "master_mgr.h"
#include "util.h"

namespace paxos {

MasterMgr::MasterMgr(
    const Node * poPaxosNode, 
    const int iGroupIdx, 
    const LogStorage * poLogStorage,
    MasterChangeCallback pMasterChangeCallback) 
    : m_oDefaultMasterSM(poLogStorage, poPaxosNode->GetNodeId(), iGroupIdx, pMasterChangeCallback) 
{
    m_iLeaseTime = 10000;

    m_poPaxosNode = (Node *)poPaxosNode;
    m_iMyGroupIdx = iGroupIdx;
    
    m_bIsEnd = false;
    m_bIsStarted = false;
    
    m_bNeedDropMaster = false;
}

MasterMgr::~MasterMgr()
{
}

int MasterMgr::Init()
{
    return m_oDefaultMasterSM.Init();
}

void MasterMgr::SetLeaseTime(const int iLeaseTimeMs)
{
    if (iLeaseTimeMs < 1000)
    {
        return;
    }

    m_iLeaseTime = iLeaseTimeMs;
}

void MasterMgr::DropMaster()
{
    m_bNeedDropMaster = true;
}

void MasterMgr::StopMaster()
{
    if (m_bIsStarted)
    {
        m_bIsEnd = true;
        Join();
    }
}

void MasterMgr::RunMaster()
{
    Start();
}

void MasterMgr::Run()
{
    m_bIsStarted = true;

    while(true)
    {
        if (m_bIsEnd)
        {
            return;
        }
        
        int iLeaseTime = m_iLeaseTime;

        uint64_t llBeginTime = Time::GetSteadyClockMS();
        
        TryBeMaster(iLeaseTime);

        int iContinueLeaseTimeout = (iLeaseTime - 100) / 4;
        iContinueLeaseTimeout = iContinueLeaseTimeout / 2 + Util::FastRand() % iContinueLeaseTimeout;

        if (m_bNeedDropMaster)
        {
            //BP->GetMasterBP()->DropMaster();
            m_bNeedDropMaster = false;
            iContinueLeaseTimeout = iLeaseTime * 2;
            //PLG1Imp("Need drop master, this round wait time %dms", iContinueLeaseTimeout);
        }
        
        uint64_t llEndTime = Time::GetSteadyClockMS();
        int iRunTime = llEndTime > llBeginTime ? llEndTime - llBeginTime : 0;
        int iNeedSleepTime = iContinueLeaseTimeout > iRunTime ? iContinueLeaseTimeout - iRunTime : 0;

        //PLG1Imp("TryBeMaster, sleep time %dms", iNeedSleepTime);
        Time::MsSleep(iNeedSleepTime);
    }
}

void MasterMgr::TryBeMaster(const int iLeaseTime)
{
    uint64_t iMasterNodeID = nullnode;
    uint64_t llMasterVersion = 0;

    //step 1 check exist master and get version
    m_oDefaultMasterSM.SafeGetMaster(iMasterNodeID, llMasterVersion);

    if (iMasterNodeID != nullnode && (iMasterNodeID != m_poPaxosNode->GetNodeId()))
    {
        return;
    }

    //BP->GetMasterBP()->TryBeMaster();

    //step 2 try be master
    std::string sPaxosValue;
    if (!MasterStateMachine::MakeOpValue(
                m_poPaxosNode->GetNodeId(),
                llMasterVersion,
                iLeaseTime,
                MasterOperatorType_Complete,
                sPaxosValue))
    {
        //PLG1Err("Make paxos value fail");
        return;
    }

    const int iMasterLeaseTimeout = iLeaseTime - 100;
    
    uint64_t llAbsMasterTimeout = Time::GetSteadyClockMS() + iMasterLeaseTimeout;
    uint64_t llCommitInstanceID = 0;

    StateMachineCtx oCtx;
    oCtx.state_machine_id_ = MASTER_V_STATE_MACHINE_ID;
    oCtx.ctx_ = (void *)&llAbsMasterTimeout;

    int ret = m_poPaxosNode->Propose(m_iMyGroupIdx, sPaxosValue, llCommitInstanceID, &oCtx);
    if (ret != 0)
    {
        //BP->GetMasterBP()->TryBeMasterProposeFail();
    }
}

MasterStateMachine * MasterMgr::GetMasterSM()
{
    return &m_oDefaultMasterSM;
}

    
}



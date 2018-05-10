#include "def.h"
#include "notifier_pool.h"
#include "paxos_msg.pb.h"
#include "propose_batch.h"
#include "state_machine_base.h"
#include "util.h"
#include <assert.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <pthread.h>

namespace paxos {

uint64_t GetThreadID() {
    return (uint64_t)pthread_self();
}

PendingProposal::PendingProposal()
    : psValue(nullptr), poStateMachineCtx(nullptr), pllInstanceID(nullptr), 
    piBatchIndex(nullptr), poNotifier(nullptr), llAbsEnqueueTime(0)
{
}

ProposeBatch::ProposeBatch(const int iGroupIdx, Node * poPaxosNode, NotifierPool * poNotifierPool)
    : m_iMyGroupIdx(iGroupIdx), m_poPaxosNode(poPaxosNode), 
    m_poNotifierPool(poNotifierPool), m_bIsEnd(false), m_bIsStarted(false), m_iNowQueueValueSize(0),
    m_iBatchCount(5), m_iBatchDelayTimeMs(20), m_iBatchMaxSize(500 * 1024),
    m_poThread(nullptr)
{
}

ProposeBatch::~ProposeBatch()
{
    delete m_poThread;
}

void ProposeBatch::Start()
{
    m_poThread = new std::thread(&ProposeBatch::Run, this);
    assert(m_poThread != nullptr);
}

void ProposeBatch::Stop()
{
    if (m_bIsStarted)
    {
        std::unique_lock<std::mutex> oLock(m_oMutex);
        m_bIsEnd = true;
        m_oCond.notify_all();
        oLock.unlock();

        m_poThread->join();
    }
}

void ProposeBatch::SetBatchCount(const int iBatchCount)
{
    m_iBatchCount = iBatchCount;
}

void ProposeBatch::SetBatchDelayTimeMs(const int iBatchDelayTimeMs)
{
    m_iBatchDelayTimeMs = iBatchDelayTimeMs;
}

int ProposeBatch::Propose(const std::string & sValue, uint64_t & llInstanceID, uint32_t & iBatchIndex, StateMachineCtx * poStateMachineCtx)
{
    if (m_bIsEnd)
    {
        return Paxos_SystemError; 
    }

    //BP->GetCommiterBP()->BatchPropose();

    uint64_t llThreadID = GetThreadID();

    Notifier * poNotifier = nullptr;
    int ret = m_poNotifierPool->GetNotifier(llThreadID, poNotifier);
    if (ret != 0)
    {
        //BP->GetCommiterBP()->BatchProposeFail();
        return Paxos_SystemError;
    }

    AddProposal(sValue, llInstanceID, iBatchIndex, poStateMachineCtx, poNotifier);

    poNotifier->WaitNotify(ret);
    if (ret == PaxosTryCommitRet_OK)
    {
        //BP->GetCommiterBP()->BatchProposeOK();
    }
    else
    {
        //BP->GetCommiterBP()->BatchProposeFail();
    }

    return ret;
}

const bool ProposeBatch::NeedBatch()
{
    if ((int)m_oQueue.size() >= m_iBatchCount
            || m_iNowQueueValueSize >= m_iBatchMaxSize)
    {
        return true;
    }
    else if (m_oQueue.size() > 0)
    {
        PendingProposal & oPendingProposal = m_oQueue.front();
        uint64_t llNowTime = Time::GetSteadyClockMS();
        int iProposalPassTime = llNowTime > oPendingProposal.llAbsEnqueueTime ?
            llNowTime - oPendingProposal.llAbsEnqueueTime : 0;
        if (iProposalPassTime > m_iBatchDelayTimeMs)
        {
            return true;
        }
    }

    return false;
}

void ProposeBatch::AddProposal(const std::string & sValue, uint64_t & llInstanceID, uint32_t & iBatchIndex,
        StateMachineCtx * poStateMachineCtx, Notifier * poNotifier)
{
    std::unique_lock<std::mutex> oLock(m_oMutex);

    PendingProposal oPendingProposal;
    oPendingProposal.poNotifier = poNotifier;
    oPendingProposal.psValue = &sValue;
    oPendingProposal.poStateMachineCtx = poStateMachineCtx;
    oPendingProposal.pllInstanceID = &llInstanceID;
    oPendingProposal.piBatchIndex = &iBatchIndex;
    oPendingProposal.llAbsEnqueueTime = Time::GetSteadyClockMS();

    m_oQueue.push(oPendingProposal);
    m_iNowQueueValueSize += (int)oPendingProposal.psValue->size();

    if (NeedBatch())
    {
        std::vector<PendingProposal> vecRequest;
        PluckProposal(vecRequest);

        oLock.unlock();
        DoPropose(vecRequest);
    }
}

void ProposeBatch::Run()
{
    m_bIsStarted = true;
    //daemon thread for very low qps.
    TimeStat oTimeStat;
    while (true)
    {
        std::unique_lock<std::mutex> oLock(m_oMutex);

        if (m_bIsEnd)
        {
            break;
        }

        oTimeStat.Point();

        std::vector<PendingProposal> vecRequest;
        PluckProposal(vecRequest);

        oLock.unlock();
        DoPropose(vecRequest);
        oLock.lock();

        int iPassTime = oTimeStat.Point();
        int iNeedSleepTime = iPassTime < m_iBatchDelayTimeMs ?
            m_iBatchDelayTimeMs - iPassTime : 0;

        if (NeedBatch())
        {
            iNeedSleepTime = 0;
        }

        if (iNeedSleepTime > 0)
        {
            m_oCond.wait_for(oLock, std::chrono::milliseconds(iNeedSleepTime));
        }

        //PLG1Debug("one loop, sleep time %dms", iNeedSleepTime);
    }

    //notify all waiting thread.
    std::unique_lock<std::mutex> oLock(m_oMutex);
    while (!m_oQueue.empty())
    {
        PendingProposal & oPendingProposal = m_oQueue.front();
        oPendingProposal.poNotifier->SendNotify(Paxos_SystemError);
        m_oQueue.pop();
    }
}

void ProposeBatch::PluckProposal(std::vector<PendingProposal> & vecRequest)
{
    int iPluckCount = 0;
    int iPluckSize = 0;

    uint64_t llNowTime = Time::GetSteadyClockMS();

    while (!m_oQueue.empty())
    {
        PendingProposal & oPendingProposal = m_oQueue.front();
        vecRequest.push_back(oPendingProposal);

        iPluckCount++;
        iPluckSize += oPendingProposal.psValue->size();
        m_iNowQueueValueSize -= oPendingProposal.psValue->size();

        {
            int iProposalWaitTime = llNowTime > oPendingProposal.llAbsEnqueueTime ?
                llNowTime - oPendingProposal.llAbsEnqueueTime : 0;
            //BP->GetCommiterBP()->BatchProposeWaitTimeMs(iProposalWaitTime);
        }

        m_oQueue.pop();

        if (iPluckCount >= m_iBatchCount
                || iPluckSize >= m_iBatchMaxSize)
        {
            break;
        }
    }

    if (vecRequest.size() > 0)
    {
        //PLG1Debug("pluck %zu request", vecRequest.size());
    }
}

void ProposeBatch::OnlyOnePropose(PendingProposal & oPendingProposal)
{
    int ret = m_poPaxosNode->Propose(
        m_iMyGroupIdx, 
        *oPendingProposal.psValue, 
        *oPendingProposal.pllInstanceID, 
        oPendingProposal.poStateMachineCtx
        );
    oPendingProposal.poNotifier->SendNotify(ret);
}

void ProposeBatch::DoPropose(std::vector<PendingProposal> & vecRequest)
{
    if (vecRequest.size() == 0)
    {
        return;
    }

    //BP->GetCommiterBP()->BatchProposeDoPropose((int)vecRequest.size());

    if (vecRequest.size() == 1)
    {
        OnlyOnePropose(vecRequest[0]);
        return;
    }

    BatchPaxosValues oBatchValues;
    BatchStateMachineCtx oBatchStateMachineCtx;
    for (auto & oPendingProposal : vecRequest)
    {
        PaxosValue * poValue = oBatchValues.add_values();
        poValue->set_smid(oPendingProposal.poStateMachineCtx != nullptr ? oPendingProposal.poStateMachineCtx->state_machine_id_ : 0);
        poValue->set_value(*oPendingProposal.psValue);

        oBatchStateMachineCtx.vec_state_machine_ctx_list_.push_back(oPendingProposal.poStateMachineCtx);
    }

    StateMachineCtx oCtx;
    oCtx.state_machine_id_ = BATCH_PROPOSE_STATE_MACHINE_ID;
    oCtx.ctx_ = (void *)&oBatchStateMachineCtx;

    std::string sBuffer;
    uint64_t llInstanceID = 0;
    int ret = 0;
    bool bSucc = oBatchValues.SerializeToString(&sBuffer);
    if (bSucc)
    {
        ret = m_poPaxosNode->Propose(m_iMyGroupIdx, sBuffer, llInstanceID, &oCtx);
        if (ret != 0)
        {
            //PLG1Err("real propose fail, ret %d", ret);
        }
    }
    else
    {
        //PLG1Err("BatchValues SerializeToString fail");
        ret = Paxos_SystemError;
    }

    for (size_t i = 0; i < vecRequest.size(); i++)
    {
        PendingProposal & oPendingProposal = vecRequest[i];
        *oPendingProposal.piBatchIndex = (uint32_t)i;
        *oPendingProposal.pllInstanceID = llInstanceID; 
        oPendingProposal.poNotifier->SendNotify(ret);
    }
}

}


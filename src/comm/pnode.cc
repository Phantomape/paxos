#include "pnode.h"

namespace paxos
{

PNode::PNode()
    : m_iMyNodeID(nullnode)
{
}

PNode::~PNode()
{
    //1.step: must stop master(app) first.
    for (auto & poMaster : m_vecMasterList)
    {
        poMaster->StopMaster();
    }

    //2.step: stop proposebatch
    for (auto & poProposeBatch : m_vecProposeBatch)
    {
        poProposeBatch->Stop();
    }

    //3.step: stop group
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->Stop();
    }

    //4. step: stop network.
    m_oDefaultNetwork.Stop();

    //5 .step: delete paxos instance.
    for (auto & poGroup : m_vecGroupList)
    {
        delete poGroup;
    }

    //6. step: delete master state machine.
    for (auto & poMaster : m_vecMasterList)
    {
        delete poMaster;
    }

    //7. step: delete proposebatch;
    for (auto & poProposeBatch : m_vecProposeBatch)
    {
        delete poProposeBatch;
    }
}

int PNode::InitLogStorage(const Options & oOptions, LogStorage *& poLogStorage)
{
    if (oOptions.log_storage_ != nullptr)
    {
        poLogStorage = oOptions.log_storage_;
        //PLImp("OK, use user logstorage");
        return 0;
    }

    if (oOptions.log_storage_path_.size() == 0)
    {
        //PLErr("LogStorage Path is null");
        return -2;
    }

    int ret = m_oDefaultLogStorage.Init(oOptions.log_storage_path_, oOptions.group_count_);
    if (ret != 0)
    {
        return ret;
    }

    poLogStorage = &m_oDefaultLogStorage;
    
    //PLImp("OK, use default logstorage");

    return 0;
}

int PNode::InitNetwork(const Options & oOptions, Network *& poNetwork)
{
    if (oOptions.network_ != nullptr)
    {
        poNetwork = oOptions.network_;
        //PLImp("OK, use user network");
        return 0;
    }

    int ret = m_oDefaultNetwork.Init(
            oOptions.node_.GetIp(), oOptions.node_.GetPort(), oOptions.io_thread_count_);
    if (ret != 0)
    {
        return ret;
    }

    poNetwork = &m_oDefaultNetwork;
    
    //PLImp("OK, use default network");

    return 0;
}

int PNode::CheckOptions(const Options & oOptions)
{
    //init logger
    if (oOptions.log_storage_ != nullptr)
    {
        //LOGGER->SetLogFunc(oOptions.pLogFunc);
    }
    else
    {
        //LOGGER->InitLogger(oOptions.eLogLevel);
    }
    
    if (oOptions.log_storage_ == nullptr && oOptions.log_storage_path_.size() == 0)
    {
        //PLErr("no logpath and logstorage is null");
        return -2;
    }

    if (oOptions.udp_max_msg_size_ > 64 * 1024)
    {
        //PLErr("udp max size %zu is too large", oOptions.iUDPMaxSize);
        return -2;
    }

    if (oOptions.group_count_ > 200)
    {
        //PLErr("group count %d is too large", oOptions.group_count_);
        return -2;
    }

    if (oOptions.group_count_ <= 0)
    {
        //PLErr("group count %d is small than zero or equal to zero", oOptions.group_count_);
        return -2;
    }
    
    for (auto & oFollowerNodeInfo : oOptions.vec_follower_node_info_list_)
    {
        if (oFollowerNodeInfo.node_.GetNodeId() == oFollowerNodeInfo.follow_node_.GetNodeId())
        {
            return -2;
        }
    }

    for (auto & oGroupSMInfo : oOptions.vec_group_state_machine_info_list_)
    {
        if (oGroupSMInfo.group_idx_ >= oOptions.group_count_)
        {
            return -2;
        }
    }

    return 0;
}

void PNode::InitStateMachine(const Options & oOptions)
{
    for (auto & oGroupSMInfo : oOptions.vec_group_state_machine_info_list_)
    {
        for (auto & poSM : oGroupSMInfo.vec_state_machine_list_)
        {
            AddStateMachine(oGroupSMInfo.group_idx_, poSM);
        }
    }
}

void PNode::RunMaster(const Options & oOptions)
{
    for (auto & oGroupSMInfo : oOptions.vec_group_state_machine_info_list_)
    {
        //check if need to run master.
        if (oGroupSMInfo.use_master_)
        {
            if (!m_vecGroupList[oGroupSMInfo.group_idx_]->GetConfig()->IsIMFollower())
            {
                m_vecMasterList[oGroupSMInfo.group_idx_]->RunMaster();
            }
            else
            {
                //PLImp("I'm follower, not run master damon.");
            }
        }
    }
}

void PNode::RunProposeBatch()
{
    for (auto & poProposeBatch : m_vecProposeBatch)
    {
        poProposeBatch->Start();
    }
}

int PNode::Init(const Options & oOptions, Network *& poNetwork)
{
    int ret = CheckOptions(oOptions);
    if (ret != 0)
    {
        //PLErr("CheckOptions fail, ret %d", ret);
        return ret;
    }

    m_iMyNodeID = oOptions.node_.GetNodeId();

    //step1 init logstorage
    LogStorage * poLogStorage = nullptr;
    ret = InitLogStorage(oOptions, poLogStorage);
    if (ret != 0)
    {
        return ret;
    }

    //step2 init network
    ret = InitNetwork(oOptions, poNetwork);
    if (ret != 0)
    {
        return ret;
    }

    //step3 build masterlist
    for (int iGroupIdx = 0; iGroupIdx < oOptions.group_count_; iGroupIdx++)
    {
        MasterMgr * poMaster = new MasterMgr(this, iGroupIdx, poLogStorage, oOptions.master_change_callback_);
        assert(poMaster != nullptr);
        m_vecMasterList.push_back(poMaster);

        ret = poMaster->Init();
        if (ret != 0)
        {
            return ret;
        }
    }

    //step4 build grouplist
    for (int iGroupIdx = 0; iGroupIdx < oOptions.group_count_; iGroupIdx++)
    {
        Group * poGroup = new Group(poLogStorage, poNetwork, m_vecMasterList[iGroupIdx]->GetMasterSM(), iGroupIdx, oOptions);
        assert(poGroup != nullptr);
        m_vecGroupList.push_back(poGroup);
    }

    //step5 build batchpropose
    if (oOptions.use_batch_propose_)
    {
        for (int iGroupIdx = 0; iGroupIdx < oOptions.group_count_; iGroupIdx++)
        {
            ProposeBatch * poProposeBatch = new ProposeBatch(iGroupIdx, this, &m_oNotifierPool);
            assert(poProposeBatch != nullptr);
            m_vecProposeBatch.push_back(poProposeBatch);
        }
    }

    //step6 init statemachine
    InitStateMachine(oOptions);    

    //step7 parallel init group
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->StartInit();
    }

    for (auto & poGroup : m_vecGroupList)
    {
        int initret = poGroup->GetInitRet();
        if (initret != 0)
        {
            ret = initret;
        }
    }

    if (ret != 0)
    {
        return ret;
    }

    //last step. must init ok, then should start threads.
    //because that stop threads is slower, if init fail, we need much time to stop many threads.
    //so we put start threads in the last step.
    for (auto & poGroup : m_vecGroupList)
    {
        //start group's thread first.
        poGroup->Start();
    }
    RunMaster(oOptions);
    RunProposeBatch();

    //PLHead("OK");

    return 0;
}

bool PNode::CheckGroupID(const int iGroupIdx)
{
    if (iGroupIdx < 0 || iGroupIdx >= (int)m_vecGroupList.size())
    {
        return false;
    }

    return true;
}

int PNode::Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetCommitter()->NewValueGetID(sValue, llInstanceID);
}

int PNode::Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, StateMachineCtx * poStateMachineCtx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetCommitter()->NewValueGetID(sValue, llInstanceID, poStateMachineCtx);
}

const uint64_t PNode::GetCurrentInstanceId(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return (uint64_t)-1;
    }

    return m_vecGroupList[iGroupIdx]->GetInstance()->GetInstanceId();
}

const uint64_t PNode::GetMinChosenInstanceId(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return (uint64_t)-1;
    }

    return m_vecGroupList[iGroupIdx]->GetInstance()->GetMinChosenInstanceId();
}

int PNode::OnReceiveMessage(const char * pcMessage, const int iMessageLen)
{
    if (pcMessage == nullptr || iMessageLen <= 0)
    {
        //PLErr("Message size %d to small, not valid.", iMessageLen);
        return -2;
    }
    
    int iGroupIdx = -1;

    memcpy(&iGroupIdx, pcMessage, GROUPIDXLEN);

    if (!CheckGroupID(iGroupIdx))
    {
        //PLErr("Message groupid %d wrong, groupsize %zu", iGroupIdx, m_vecGroupList.size());
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetInstance()->OnReceiveMessage(pcMessage, iMessageLen);
}

void PNode::AddStateMachine(StateMachine * poSM)
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->AddStateMachine(poSM);
    }
}

void PNode::AddStateMachine(const int iGroupIdx, StateMachine * poSM)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return;
    }
    
    m_vecGroupList[iGroupIdx]->AddStateMachine(poSM);
}

const uint64_t PNode::GetNodeId() const
{
    return m_iMyNodeID;
}

void PNode::SetTimeoutMs(const int iTimeoutMs)
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCommitter()->SetTimeoutMs(iTimeoutMs);
    }
}

////////////////////////////////////////////////////////////////////////

void PNode::SetHoldPaxosLogCount(const uint64_t llHoldCount)
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointCleaner()->SetHoldPaxosLogCount(llHoldCount);
    }
}

void PNode::PauseCheckpointReplayer()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointReplayer()->Pause();
    }
}

void PNode::ContinueCheckpointReplayer()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointReplayer()->Continue();
    }
}

void PNode::PausePaxosLogCleaner()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointCleaner()->Pause();
    }
}

void PNode::ContinuePaxosLogCleaner()
{
    for (auto & poGroup : m_vecGroupList)
    {
        poGroup->GetCheckpointCleaner()->Continue();
    }
}

///////////////////////////////////////////////////////

int PNode::ProposalMembership(
        SystemVSM * poSystemVSM, 
        const int iGroupIdx, 
        const NodeInfoList & vecNodeInfoList, 
        const uint64_t llVersion)
{
    std::string sOpValue;
    int ret = poSystemVSM->Membership_OPValue(vecNodeInfoList, llVersion, sOpValue);
    if (ret != 0)
    {
        return Paxos_SystemError;
    }

    StateMachineCtx oCtx;
    int smret = -1;
    oCtx.state_machine_id_ = SYSTEM_V_STATE_MACHINE_ID;
    oCtx.ctx_ = (void *)&smret;

    uint64_t llInstanceID = 0;
    ret = Propose(iGroupIdx, sOpValue, llInstanceID, &oCtx);
    if (ret != 0)
    {
        return ret;
    }

    return smret;
}

int PNode::AddMember(const int iGroupIdx, const NodeInfo & oNode)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    SystemVSM * poSystemVSM = m_vecGroupList[iGroupIdx]->GetConfig()->GetSystemVSM();

    if (poSystemVSM->GetGid() == 0)
    {
        return Paxos_MembershipOp_NoGid;
    }

    uint64_t llVersion = 0;
    NodeInfoList vecNodeInfoList;
    poSystemVSM->GetMembership(vecNodeInfoList, llVersion);

    for (auto & oNodeInfo : vecNodeInfoList)
    {
        if (oNodeInfo.GetNodeId() == oNode.GetNodeId())
        {
            return Paxos_MembershipOp_Add_NodeExist;
        }
    }

    vecNodeInfoList.push_back(oNode);

    return ProposalMembership(poSystemVSM, iGroupIdx, vecNodeInfoList, llVersion);
}

int PNode::RemoveMember(const int iGroupIdx, const NodeInfo & oNode)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    SystemVSM * poSystemVSM = m_vecGroupList[iGroupIdx]->GetConfig()->GetSystemVSM();

    if (poSystemVSM->GetGid() == 0)
    {
        return Paxos_MembershipOp_NoGid;
    }

    uint64_t llVersion = 0;
    NodeInfoList vecNodeInfoList;
    poSystemVSM->GetMembership(vecNodeInfoList, llVersion);

    bool bNodeExist = false;
    NodeInfoList vecAfterNodeInfoList;
    for (auto & oNodeInfo : vecNodeInfoList)
    {
        if (oNodeInfo.GetNodeId() == oNode.GetNodeId())
        {
            bNodeExist = true;
        }
        else
        {
            vecAfterNodeInfoList.push_back(oNodeInfo);
        }
    }

    if (!bNodeExist)
    {
        return Paxos_MembershipOp_Remove_NodeNotExist;
    }

    return ProposalMembership(poSystemVSM, iGroupIdx, vecAfterNodeInfoList, llVersion);
}

int PNode::ChangeMember(const int iGroupIdx, const NodeInfo & oFromNode, const NodeInfo & oToNode)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    SystemVSM * poSystemVSM = m_vecGroupList[iGroupIdx]->GetConfig()->GetSystemVSM();

    if (poSystemVSM->GetGid() == 0)
    {
        return Paxos_MembershipOp_NoGid;
    }

    uint64_t llVersion = 0;
    NodeInfoList vecNodeInfoList;
    poSystemVSM->GetMembership(vecNodeInfoList, llVersion);

    NodeInfoList vecAfterNodeInfoList;
    bool bFromNodeExist = false;
    bool bToNodeExist = false;
    for (auto & oNodeInfo : vecNodeInfoList)
    {
        if (oNodeInfo.GetNodeId() == oFromNode.GetNodeId())
        {
            bFromNodeExist = true;
            continue;
        }
        else if (oNodeInfo.GetNodeId() == oToNode.GetNodeId())
        {
            bToNodeExist = true;
            continue;
        }

        vecAfterNodeInfoList.push_back(oNodeInfo);
    }

    if ((!bFromNodeExist) && bToNodeExist)
    {
        return Paxos_MembershipOp_Change_NoChange;
    }

    vecAfterNodeInfoList.push_back(oToNode);

    return ProposalMembership(poSystemVSM, iGroupIdx, vecAfterNodeInfoList, llVersion);
}

int PNode::ShowMembership(const int iGroupIdx, NodeInfoList & vecNodeInfoList)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    SystemVSM * poSystemVSM = m_vecGroupList[iGroupIdx]->GetConfig()->GetSystemVSM();

    uint64_t llVersion = 0;
    poSystemVSM->GetMembership(vecNodeInfoList, llVersion);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////

const NodeInfo PNode::GetMaster(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return NodeInfo(nullnode);
    }

    return NodeInfo(m_vecMasterList[iGroupIdx]->GetMasterSM()->GetMaster());
}

const NodeInfo PNode::GetMasterWithVersion(const int iGroupIdx, uint64_t & llVersion) 
{
    if (!CheckGroupID(iGroupIdx))
    {
        return NodeInfo(nullnode);
    }

    return NodeInfo(m_vecMasterList[iGroupIdx]->GetMasterSM()->GetMasterWithVersion(llVersion));
}

const bool PNode::IsIMMaster(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return false;
    }

    return m_vecMasterList[iGroupIdx]->GetMasterSM()->IsIMMaster();
}

int PNode::SetMasterLease(const int iGroupIdx, const int iLeaseTimeMs)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    m_vecMasterList[iGroupIdx]->SetLeaseTime(iLeaseTimeMs);
    return 0;
}

int PNode::DropMaster(const int iGroupIdx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    m_vecMasterList[iGroupIdx]->DropMaster();
    return 0;
}

/////////////////////////////////////////////////////////////////////

void PNode::SetMaxHoldThreads(const int iGroupIdx, const int iMaxHoldThreads)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return;
    }

    m_vecGroupList[iGroupIdx]->GetCommitter()->SetMaxHoldThreads(iMaxHoldThreads);
}

void PNode::SetProposeWaitTimeThresholdMS(const int iGroupIdx, const int iWaitTimeThresholdMS)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return;
    }

    m_vecGroupList[iGroupIdx]->GetCommitter()->SetProposeWaitTimeThresholdMS(iWaitTimeThresholdMS);
}

void PNode::SetLogSync(const int iGroupIdx, const bool bLogSync)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return;
    }

    m_vecGroupList[iGroupIdx]->GetConfig()->SetLogSync(bLogSync);
}

//////////////////////////////////////////////////////////////////////

int PNode::GetInstanceValue(const int iGroupIdx, const uint64_t llInstanceID, 
        std::vector<std::pair<std::string, int> > & vecValues)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    std::string sValue;
    int iSMID = 0;
    int ret = m_vecGroupList[iGroupIdx]->GetInstance()->GetInstanceValue(llInstanceID, sValue, iSMID);
    if (ret != 0)
    {
        return ret;
    }

    if (iSMID == BATCH_PROPOSE_STATE_MACHINE_ID)
    {
        BatchPaxosValues oBatchValues;
        bool bSucc = oBatchValues.ParseFromArray(sValue.data(), sValue.size());
        if (!bSucc)
        {
            return Paxos_SystemError;
        }

        for (int i = 0; i < oBatchValues.values_size(); i++)
        {
            const PaxosValue & oValue = oBatchValues.values(i);
            vecValues.push_back(make_pair(oValue.value(), oValue.smid()));
        }
    }
    else
    {
        vecValues.push_back(make_pair(sValue, iSMID));
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////

int PNode::BatchPropose(const int iGroupIdx, const std::string & sValue, 
        uint64_t & llInstanceID, uint32_t & iBatchIndex)
{
    return BatchPropose(iGroupIdx, sValue, llInstanceID, iBatchIndex, nullptr);
}

int PNode::BatchPropose(const int iGroupIdx, const std::string & sValue, 
        uint64_t & llInstanceID, uint32_t & iBatchIndex, StateMachineCtx * poStateMachineCtx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    if (m_vecProposeBatch.size() == 0)
    {
        return Paxos_SystemError;
    }

    return m_vecProposeBatch[iGroupIdx]->Propose(sValue, llInstanceID, iBatchIndex, poStateMachineCtx);
}

void PNode::SetBatchCount(const int iGroupIdx, const int iBatchCount)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return;
    }

    if (m_vecProposeBatch.size() == 0)
    {
        return;
    }

    m_vecProposeBatch[iGroupIdx]->SetBatchCount(iBatchCount);
}

void PNode::SetBatchDelayTimeMs(const int iGroupIdx, const int iBatchDelayTimeMs)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return;
    }

    if (m_vecProposeBatch.size() == 0)
    {
        return;
    }

    m_vecProposeBatch[iGroupIdx]->SetBatchDelayTimeMs(iBatchDelayTimeMs);
}
    
}



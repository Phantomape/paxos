#include "def.h"
#include "instance.h"
#include "internal_options.h"
#include "acceptor.h"
#include "learner.h"
#include "proposer.h"

namespace paxos {

Instance::Instance(
        const Config * poConfig,
        const LogStorage * poLogStorage,
        const Communicate * poCommunicate,
        const Options & oOptions)
    : m_oSMFac(poConfig->GetMyGroupIdx()),
    m_oIOLoop((Config *)poConfig, this),
    m_oAcceptor(poConfig, poCommunicate, this, poLogStorage),
    m_oLearner(poConfig, poCommunicate, this, &m_oAcceptor, poLogStorage, &m_oIOLoop, &m_oCheckpointMgr, &m_oSMFac),
    m_oProposer(poConfig, poCommunicate, this, &m_oLearner, &m_oIOLoop),
    m_oPaxosLog(poLogStorage),
    m_oCommitCtx((Config *)poConfig),
    m_oCommitter((Config *)poConfig, &m_oCommitCtx, &m_oIOLoop, &m_oSMFac),
    m_oCheckpointMgr((Config *)poConfig, &m_oSMFac, (LogStorage *)poLogStorage, oOptions.use_checkpoint_replayer_),
    m_oOptions(oOptions), m_bStarted(false) {
    m_poConfig = (Config *)poConfig;
    m_poCommunicate = (Communicate *)poCommunicate;
    m_iCommitTimerID = 0;
    m_iLastChecksum = 0;
}

Instance::~Instance() {
    //PLGHead("Instance Deleted, GroupIdx %d.", m_poConfig->GetMyGroupIdx());
}

int Instance::Init()
{
    //Must init acceptor first, because the max instanceid is record in acceptor state.
    int ret = m_oAcceptor.Init();
    if (ret != 0) {
        //PLGErr("Acceptor.Init fail, ret %d", ret);
        return ret;
    }

    ret = m_oCheckpointMgr.Init();
    if (ret != 0) {
        //PLGErr("CheckpointMgr.Init fail, ret %d", ret);
        return ret;
    }

    uint64_t llCPInstanceID = m_oCheckpointMgr.GetCheckpointInstanceID() + 1;

    uint64_t llNowInstanceID = llCPInstanceID;
    if (llNowInstanceID < m_oAcceptor.GetInstanceId()) {
        ret = PlayLog(llNowInstanceID, m_oAcceptor.GetInstanceId());
        if (ret != 0) {
            return ret;
        }
        llNowInstanceID = m_oAcceptor.GetInstanceId();
    }
    else {
        if (llNowInstanceID > m_oAcceptor.GetInstanceId()) {
            ret = ProtectionLogic_IsCheckpointInstanceIDCorrect(llNowInstanceID, m_oAcceptor.GetInstanceId());
            if (ret != 0) {
                return ret;
            }
            m_oAcceptor.InitInstance();
        }
        m_oAcceptor.SetInstanceId(llNowInstanceID);
    }

    m_oLearner.SetInstanceId(llNowInstanceID);
    m_oProposer.SetInstanceId(llNowInstanceID);
    //m_oProposer.SetStartProposalID(m_oAcceptor.GetAcceptorState()->GetPromiseBallot().m_llProposalID + 1);

    m_oCheckpointMgr.SetMaxChosenInstanceID(llNowInstanceID);

    ret = InitLastCheckSum();
    if (ret != 0) {
        return ret;
    }

    //m_oLearner.Reset_AskforLearn_Noop();

    //PLGImp("OK");

    return 0;
}

void Instance::Start() {
    //start learner sender
    //m_oLearner.StartLearnerSender();
    //start ioloop
    //m_oIOLoop.start();
    //start checkpoint replayer and cleaner
    m_oCheckpointMgr.Start();

    m_bStarted = true;
}

void Instance::Stop() {
    if (m_bStarted) {
        m_oIOLoop.Stop();
        m_oCheckpointMgr.Stop();
        m_oLearner.Stop();
    }
}

int Instance::ProtectionLogic_IsCheckpointInstanceIDCorrect(const uint64_t llCPInstanceID, const uint64_t llLogMaxInstanceID) {
    if (llCPInstanceID <= llLogMaxInstanceID + 1) {
        return 0;
    }

    //checkpoint_instanceid larger than log_maxinstanceid+1 will appear in the following situations
    //1. Pull checkpoint from other node automatically and restart. (normal case)
    //2. Paxos log was manually all deleted. (may be normal case)
    //3. Paxos log is lost because Options::bSync set as false. (bad case)
    //4. Checkpoint data corruption results an error checkpoint_instanceid. (bad case)
    //5. Checkpoint data copy from other node manually. (bad case)
    //In these bad cases, paxos log between [log_maxinstanceid, checkpoint_instanceid) will not exist
    //and checkpoint data maybe wrong, we can't ensure consistency in this case.

    if (llLogMaxInstanceID == 0)
    {
        //case 1. Automatically pull checkpoint will delete all paxos log first.
        //case 2. No paxos log.
        //If minchosen instanceid < checkpoint instanceid.
        //Then Fix minchosen instanceid to avoid that paxos log between [log_maxinstanceid, checkpoint_instanceid) not exist.
        //if minchosen isntanceid > checkpoint.instanceid.
        //That probably because the automatic pull checkpoint did not complete successfully.
        uint64_t llMinChosenInstanceID = m_oCheckpointMgr.GetMinChosenInstanceId();
        if (m_oCheckpointMgr.GetMinChosenInstanceId() != llCPInstanceID)
        {
            int ret = m_oCheckpointMgr.SetMinChosenInstanceID(llCPInstanceID);
            if (ret != 0)
            {
                //PLGErr("SetMinChosenInstanceID fail, now minchosen %lu max instanceid %lu checkpoint instanceid %lu",
                //        m_oCheckpointMgr.GetMinChosenInstanceId(), llLogMaxInstanceID, llCPInstanceID);
                return -1;
            }

            //PLGStatus("Fix minchonse instanceid ok, old minchosen %lu now minchosen %lu max %lu checkpoint %lu",
            //        llMinChosenInstanceID, m_oCheckpointMgr.GetMinChosenInstanceId(),
            //        llLogMaxInstanceID, llCPInstanceID);
        }

        return 0;
    }
    else
    {
        //other case.
        //PLGErr("checkpoint instanceid %lu larger than log max instanceid %lu. "
        //        "Please ensure that your checkpoint data is correct. "
        //        "If you ensure that, just delete all paxos log data and restart.",
        //        llCPInstanceID, llLogMaxInstanceID);
        return -2;
    }
}

int Instance::InitLastCheckSum()
{
    if (m_oAcceptor.GetInstanceId() == 0)
    {
        m_iLastChecksum = 0;
        return 0;
    }

    if (m_oAcceptor.GetInstanceId() <= m_oCheckpointMgr.GetMinChosenInstanceId())
    {
        m_iLastChecksum = 0;
        return 0;
    }

    AcceptorStateData oState;
    int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), m_oAcceptor.GetInstanceId() - 1, oState);
    if (ret != 0 && ret != 1)
    {
        return ret;
    }

    if (ret == 1)
    {
        //PLGErr("las checksum not exist, now instanceid %lu", m_oAcceptor.GetInstanceId());
        m_iLastChecksum = 0;
        return 0;
    }

    m_iLastChecksum = oState.checksum();

    //PLGImp("ok, last checksum %u", m_iLastChecksum);

    return 0;
}

int Instance::PlayLog(const uint64_t llBeginInstanceID, const uint64_t llEndInstanceID) {
    if (llBeginInstanceID < m_oCheckpointMgr.GetMinChosenInstanceId()) {
        //PLGErr("now instanceid %lu small than min chosen instanceid %lu",
        //        llBeginInstanceID, m_oCheckpointMgr.GetMinChosenInstanceId());
        return -2;
    }

    for (uint64_t llInstanceID = llBeginInstanceID; llInstanceID < llEndInstanceID; llInstanceID++) {
        AcceptorStateData oState;
        int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), llInstanceID, oState);
        if (ret != 0) {
            //PLGErr("log read fail, instanceid %lu ret %d", llInstanceID, ret);
            return ret;
        }

        bool bExecuteRet = m_oSMFac.Execute(m_poConfig->GetMyGroupIdx(), llInstanceID, oState.acceptedvalue(), nullptr);
        if (!bExecuteRet)
        {
            //PLGErr("Execute fail, instanceid %lu", llInstanceID);
            return -1;
        }
    }

    return 0;
}

const uint32_t Instance::GetLastChecksum()
{
    return m_iLastChecksum;
}

Committer * Instance::GetCommitter()
{
    return &m_oCommitter;
}

Cleaner * Instance::GetCheckpointCleaner()
{
    return m_oCheckpointMgr.GetCleaner();
}

Replayer * Instance::GetCheckpointReplayer()
{
    return m_oCheckpointMgr.GetReplayer();
}

////////////////////////////////////////////////

void Instance::CheckNewValue()
{
    /*
    if (!m_oLearner.IsIMLatest())
    {
        return;
    }
    */

    if (m_poConfig->IsIMFollower())
    {
        //PLGErr("I'm follower, skip this new value");
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Follower_Cannot_Commit);
        return;
    }

    if (!m_poConfig->CheckConfig())
    {
        //PLGErr("I'm not in membership, skip this new value");
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Im_Not_In_Membership);
        return;
    }

    if ((int)m_oCommitCtx.GetCommitValue().size() > MAX_VALUE_SIZE)
    {
        //PLGErr("value size %zu to large, skip this new value",
        //    m_oCommitCtx.GetCommitValue().size());
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Value_Size_TooLarge);
        return;
    }

    m_oCommitCtx.StartCommit(m_oProposer.GetInstanceId());

    if (m_oCommitCtx.GetTimeoutMs() != -1) {
        m_oIOLoop.AddTimer(m_oCommitCtx.GetTimeoutMs(), Timer_Instance_Commit_Timeout, m_iCommitTimerID);
    }

    m_oTimeStat.Point();

    if (m_poConfig->GetIsUseMembership() && (m_oProposer.GetInstanceId() == 0 || m_poConfig->GetGid() == 0)) {
        //Init system variables.
        //PLGHead("Need to init system variables, Now.InstanceID %lu Now.Gid %lu",
        //        m_oProposer.GetInstanceId(), m_poConfig->GetGid());

        uint64_t llGid = Util::GenGid(m_poConfig->GetMyNodeID());
        std::string sValue;
        int ret = m_poConfig->GetSystemVSM()->CreateGid_OPValue(llGid, sValue);
        assert(ret == 0);

        m_oSMFac.PackPaxosValue(sValue, m_poConfig->GetSystemVSM()->StateMachineId());
        //m_oProposer.Propose(sValue);
    }
    else {
        if (m_oOptions.open_change_value_before_propose_) {
            m_oSMFac.BeforePropose(m_poConfig->GetMyGroupIdx(), m_oCommitCtx.GetCommitValue());
        }
        //m_oProposer.Propose(m_oCommitCtx.GetCommitValue());
    }
}

void Instance::OnNewValueCommitTimeout()
{
    ////BP->GetInstance//BP()->OnProposeCommitTimeout();

    //m_oProposer.ExitPrepare();
    //m_oProposer.ExitAccept();

    //m_oCommitCtx.SetResult(PaxosTryCommitRet_Timeout, m_oProposer.GetInstanceId(), "");
}

//////////////////////////////////////////////////////////////////////

int Instance::OnReceiveMessage(const char * pcMessage, const int iMessageLen)
{
    m_oIOLoop.AddMessage(pcMessage, iMessageLen);

    return 0;
}

bool Instance::ReceiveMsgHeaderCheck(const Header & oHeader, const uint64_t iFromNodeID)
{
    if (m_poConfig->GetGid() == 0 || oHeader.gid() == 0)
    {
        return true;
    }

    if (m_poConfig->GetGid() != oHeader.gid())
    {
        ////BP->GetAlgorithmBase//BP()->HeaderGidNotSame();
        //PLGErr("Header check fail, header.gid %lu config.gid %lu, msg.from_nodeid %lu",
        //        oHeader.gid(), m_poConfig->GetGid(), iFromNodeID);
        return false;
    }

    return true;
}

void Instance::OnReceive(const std::string & sBuffer)
{
    ////BP->GetInstance//BP()->OnReceive();

    if (sBuffer.size() <= 6)
    {
        //PLGErr("buffer size %zu too short", sBuffer.size());
        return;
    }

    Header oHeader;
    size_t iBodyStartPos = 0;
    size_t iBodyLen = 0;
    int ret = Base::UnPackBaseMsg(sBuffer, oHeader, iBodyStartPos, iBodyLen);
    if (ret != 0)
    {
        return;
    }

    int iCmd = oHeader.cmdid();

    if (iCmd == MsgCmd_PaxosMsg)
    {
        if (m_oCheckpointMgr.InAskforcheckpointMode())
        {
            //PLGImp("in ask for checkpoint mode, ignord paxosmsg");
            return;
        }
        
        PaxosMsg oPaxosMsg;
        bool bSucc = oPaxosMsg.ParseFromArray(sBuffer.data() + iBodyStartPos, iBodyLen);
        if (!bSucc)
        {
            ////BP->GetInstance//BP()->OnReceiveParseError();
            //PLGErr("PaxosMsg.ParseFromArray fail, skip this msg");
            return;
        }

        if (!ReceiveMsgHeaderCheck(oHeader, oPaxosMsg.nodeid()))
        {
            return;
        }
        
        OnReceivePaxosMsg(oPaxosMsg);
    }
    else if (iCmd == MsgCmd_CheckpointMsg)
    {
        CheckpointMsg oCheckpointMsg;
        bool bSucc = oCheckpointMsg.ParseFromArray(sBuffer.data() + iBodyStartPos, iBodyLen);
        if (!bSucc)
        {
            ////BP->GetInstance//BP()->OnReceiveParseError();
            //PLGErr("PaxosMsg.ParseFromArray fail, skip this msg");
            return;
        }

        if (!ReceiveMsgHeaderCheck(oHeader, oCheckpointMsg.nodeid()))
        {
            return;
        }
        
        OnReceiveCheckpointMsg(oCheckpointMsg);
    }
}

void Instance::OnReceiveCheckpointMsg(const CheckpointMsg & oCheckpointMsg)
{
    if (oCheckpointMsg.msgtype() == CheckpointMsgType_SendFile)
    {
        if (!m_oCheckpointMgr.InAskforcheckpointMode())
        {
            //PLGImp("not in ask for checkpoint mode, ignord checkpoint msg");
            return;
        }

        //m_oLearner.OnSendCheckpoint(oCheckpointMsg);
    }
    else if (oCheckpointMsg.msgtype() == CheckpointMsgType_SendFile_Ack)
    {
        //m_oLearner.OnSendCheckpointAck(oCheckpointMsg);
    }
}

int Instance::OnReceivePaxosMsg(const PaxosMsg & oPaxosMsg, const bool bIsRetry)
{
    if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply
            || oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply
            || oPaxosMsg.msgtype() == MsgType_PaxosProposal_SendNewValue)
    {
        if (!m_poConfig->IsValidNodeID(oPaxosMsg.nodeid()))
        {
            ////BP->GetInstance//BP()->OnReceivePaxosMsgNodeIDNotValid();
            //PLGErr("acceptor reply type msg, from nodeid not in my membership, skip this message");
            return 0;
        }
        
        return ReceiveMsgForProposer(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosPrepare
            || oPaxosMsg.msgtype() == MsgType_PaxosAccept)
    {
        //if my gid is zero, then this is a unknown node.
        if (m_poConfig->GetGid() == 0)
        {
            m_poConfig->AddTmpNodeOnlyForLearn(oPaxosMsg.nodeid());
        }
        
        if ((!m_poConfig->IsValidNodeID(oPaxosMsg.nodeid())))
        {
            m_poConfig->AddTmpNodeOnlyForLearn(oPaxosMsg.nodeid());

            return 0;
        }

        ChecksumLogic(oPaxosMsg);
        return ReceiveMsgForAcceptor(oPaxosMsg, bIsRetry);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforLearn
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_ProposerSendSuccess
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_ComfirmAskforLearn
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendNowInstanceID
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue_Ack
            || oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforCheckpoint)
    {
        ChecksumLogic(oPaxosMsg);
        return ReceiveMsgForLearner(oPaxosMsg);
    }
    else
    {
        ////BP->GetInstance//BP()->OnReceivePaxosMsgTypeNotValid();
        //PLGErr("Invaid msgtype %d", oPaxosMsg.msgtype());
    }

    return 0;
}

int Instance::ReceiveMsgForProposer(const PaxosMsg & oPaxosMsg)
{
    if (m_poConfig->IsIMFollower())
    {
        //PLGErr("I'm follower, skip this message");
        return 0;
    }

    ///////////////////////////////////////////////////////////////
    
    if (oPaxosMsg.instanceid() != m_oProposer.GetInstanceId())
    {
        if (oPaxosMsg.instanceid() + 1 == m_oProposer.GetInstanceId())
        {
            //Exipred reply msg on last instance.
            //If the response of a node is always slower than the majority node, 
            //then the message of the node is always ignored even if it is a reject reply.
            //In this case, if we do not deal with these reject reply, the node that 
            //gave reject reply will always give reject reply. 
            //This causes the node to remain in catch-up state.
            //
            //To avoid this problem, we need to deal with the expired reply.
            if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply)
            {
                //m_oProposer.OnExpiredPrepareReply(oPaxosMsg);
            }
            else if (oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply)
            {
                //m_oProposer.OnExpiredAcceptReply(oPaxosMsg);
            }
        }

        ////BP->GetInstance//BP()->OnReceivePaxosProposerMsgInotsame();
        //PLGErr("InstanceID not same, skip msg");
        return 0;
    }

    if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply)
    {
        //m_oProposer.OnPrepareReply(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply)
    {
        //m_oProposer.OnAcceptReply(oPaxosMsg);
    }

    return 0;
}

int Instance::ReceiveMsgForAcceptor(const PaxosMsg & oPaxosMsg, const bool bIsRetry)
{
    if (m_poConfig->IsIMFollower())
    {
        //PLGErr("I'm follower, skip this message");
        return 0;
    }
    
    //////////////////////////////////////////////////////////////
    
    if (oPaxosMsg.instanceid() != m_oAcceptor.GetInstanceId())
    {
        ////BP->GetInstance//BP()->OnReceivePaxosAcceptorMsgInotsame();
    }
    
    if (oPaxosMsg.instanceid() == m_oAcceptor.GetInstanceId() + 1)
    {
        //skip success message
        PaxosMsg oNewPaxosMsg = oPaxosMsg;
        oNewPaxosMsg.set_instanceid(m_oAcceptor.GetInstanceId());
        oNewPaxosMsg.set_msgtype(MsgType_PaxosLearner_ProposerSendSuccess);

        ReceiveMsgForLearner(oNewPaxosMsg);
    }
            
    if (oPaxosMsg.instanceid() == m_oAcceptor.GetInstanceId())
    {
        if (oPaxosMsg.msgtype() == MsgType_PaxosPrepare)
        {
            return m_oAcceptor.OnPrepare(oPaxosMsg);
        }
        else if (oPaxosMsg.msgtype() == MsgType_PaxosAccept)
        {
            m_oAcceptor.OnAccept(oPaxosMsg);
        }
    }
    else if ((!bIsRetry) && (oPaxosMsg.instanceid() > m_oAcceptor.GetInstanceId()))
    {
        //retry msg can't retry again.
        if (oPaxosMsg.instanceid() >= m_oLearner.GetLatestInstanceID())
        {
            if (oPaxosMsg.instanceid() < m_oAcceptor.GetInstanceId() + RETRY_QUEUE_MAX_LEN)
            {
                //need retry msg precondition
                //1. prepare or accept msg
                //2. msg.instanceid > nowinstanceid. 
                //    (if < nowinstanceid, this msg is expire)
                //3. msg.instanceid >= seen latestinstanceid. 
                //    (if < seen latestinstanceid, proposer don't need reply with this instanceid anymore.)
                //4. msg.instanceid close to nowinstanceid.
                m_oIOLoop.AddRetryPaxosMsg(oPaxosMsg);
                
                ////BP->GetInstance//BP()->OnReceivePaxosAcceptorMsgAddRetry();

                //PLGErr("InstanceID not same, get in to retry logic");
            }
            else
            {
                //retry msg not series, no use.
                m_oIOLoop.ClearRetryQueue();
            }
        }
    }

    return 0;
}

int Instance::ReceiveMsgForLearner(const PaxosMsg & oPaxosMsg)
{
    if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforLearn)
    {
        //m_oLearner.OnAskforLearn(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue)
    {
        //m_oLearner.OnSendLearnValue(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_ProposerSendSuccess)
    {
        //m_oLearner.OnProposerSendSuccess(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendNowInstanceID)
    {
        //m_oLearner.OnSendNowInstanceID(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_ComfirmAskforLearn)
    {
        //m_oLearner.OnComfirmAskForLearn(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue_Ack)
    {
        //m_oLearner.OnSendLearnValue_Ack(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforCheckpoint)
    {
        //m_oLearner.OnAskforCheckpoint(oPaxosMsg);
    }

    return 0;
}

void Instance::NewInstance() {
    m_oAcceptor.NewInstance();
    m_oLearner.NewInstance();
    m_oProposer.NewInstance();
}

const uint64_t Instance::GetInstanceId() {
    return m_oAcceptor.GetInstanceId();
}

const uint64_t Instance::GetMinChosenInstanceId() {
    //return m_oCheckpointMgr.GetMinChosenInstanceId();
}

void Instance::OnTimeout(const uint32_t iTimerID, const int iType)
{
    if (iType == Timer_Proposer_Prepare_Timeout)
    {
        m_oProposer.OnPrepareTimeout();
    }
    else if (iType == Timer_Proposer_Accept_Timeout)
    {
        m_oProposer.OnAcceptTimeout();
    }
    else if (iType == Timer_Learner_Askforlearn_noop)
    {
        //m_oLearner.AskforLearn_Noop();
    }
    else if (iType == Timer_Instance_Commit_Timeout)
    {
        //OnProposeCommitTimeout();
    }
    else
    {
        //PLGErr("unknown timer type %d, timerid %u", iType, iTimerID);
    }
}

void Instance::AddStateMachine(StateMachine * poSM) {
}

bool Instance::SMExecute(
        const uint64_t llInstanceID,
        const std::string & sValue,
        const bool bIsMyCommit,
        StateMachineCtx * poStateMachineCtx
        ) {
    return m_oSMFac.Execute(m_poConfig->GetMyGroupIdx(), llInstanceID, sValue, poStateMachineCtx);
}

void Instance::ChecksumLogic(const PaxosMsg & oPaxosMsg) {
    if (oPaxosMsg.instanceid() != m_oAcceptor.GetInstanceId()) {
        return;
    }

    if (m_oAcceptor.GetInstanceId() > 0 && GetLastChecksum() == 0) {
        //PLGErr("I have no last checksum, other last checksum %u", oPaxosMsg.lastchecksum());
        m_iLastChecksum = oPaxosMsg.lastchecksum();
        return;
    }
    assert(oPaxosMsg.lastchecksum() == GetLastChecksum());
}

int Instance::GetInstanceValue(const uint64_t llInstanceID, std::string & sValue, int & iSMID) {
    iSMID = 0;

    if (llInstanceID >= m_oAcceptor.GetInstanceId())
    {
        return Paxos_GetInstanceValue_Value_Not_Chosen_Yet;
    }

    AcceptorStateData oState;
    int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), llInstanceID, oState);
    if (ret != 0 && ret != 1)
    {
        return -1;
    }

    if (ret == 1)
    {
        return Paxos_GetInstanceValue_Value_NotExist;
    }

    memcpy(&iSMID, oState.acceptedvalue().data(), sizeof(int));
    sValue = std::string(oState.acceptedvalue().data() + sizeof(int), oState.acceptedvalue().size() - sizeof(int));

    return 0;
}

}

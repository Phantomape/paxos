#include "def.h"
#include "instance.h"
#include "internal_options.h"
#include "acceptor.h"
#include "learner.h"
#include "proposer.h"

namespace paxos {

Instance::Instance(
        const Config* config,
        const LogStorage* poLogStorage,
        const Communicate* poCommunicate,
        const Options & oOptions)
    : state_machine_fac_(config->GetMyGroupIdx()),
    ioloop_((Config*)config, this),
    acceptor_(config, poCommunicate, this, poLogStorage),
    learner_(config, poCommunicate, this, &acceptor_, poLogStorage, &ioloop_, &checkpoint_mgr_, &state_machine_fac_),
    proposer_(config, poCommunicate, this, &learner_, &ioloop_),
    paxos_log_(poLogStorage),
    commit_ctx_((Config*)config),
    commiter_((Config*)config, &commit_ctx_, &ioloop_, &state_machine_fac_),
    checkpoint_mgr_((Config*)config, &state_machine_fac_, (LogStorage*)poLogStorage, oOptions.use_checkpoint_replayer_),
    options_(oOptions), is_started_(false) {
    config_ = (Config*)config;
    communicate_ = (Communicate*)poCommunicate;
    commit_timer_id_ = 0;
    last_checksum_ = 0;
}

Instance::~Instance() {
    //PLGHead("Instance Deleted, GroupIdx %d.", config_->GetMyGroupIdx());
}

int Instance::Init()
{
    //Must init acceptor first, because the max instanceid is record in acceptor state.
    int ret = acceptor_.Init();
    if (ret != 0) {
        //PLGErr("Acceptor.Init fail, ret %d", ret);
        return ret;
    }

    ret = checkpoint_mgr_.Init();
    if (ret != 0) {
        //PLGErr("CheckpointMgr.Init fail, ret %d", ret);
        return ret;
    }

    uint64_t llCPInstanceID = checkpoint_mgr_.GetCheckpointInstanceID() + 1;

    uint64_t llNowInstanceID = llCPInstanceID;
    if (llNowInstanceID < acceptor_.GetInstanceId()) {
        ret = PlayLog(llNowInstanceID, acceptor_.GetInstanceId());
        if (ret != 0) {
            return ret;
        }
        llNowInstanceID = acceptor_.GetInstanceId();
    }
    else {
        if (llNowInstanceID > acceptor_.GetInstanceId()) {
            ret = ProtectionLogic_IsCheckpointInstanceIDCorrect(llNowInstanceID, acceptor_.GetInstanceId());
            if (ret != 0) {
                return ret;
            }
            acceptor_.InitInstance();
        }
        acceptor_.SetInstanceId(llNowInstanceID);
    }

    learner_.SetInstanceId(llNowInstanceID);
    proposer_.SetInstanceId(llNowInstanceID);
    //proposer_.SetStartProposalID(acceptor_.GetAcceptorState()->GetPromiseBallot().m_llProposalID + 1);

    checkpoint_mgr_.SetMaxChosenInstanceID(llNowInstanceID);

    ret = InitLastChecksum();
    if (ret != 0) {
        return ret;
    }

    //learner_.Reset_AskforLearn_Noop();

    //PLGImp("OK");

    return 0;
}

void Instance::Start() {
    //start learner sender
    //learner_.StartLearnerSender();
    //start ioloop
    //ioloop_.start();
    //start checkpoint replayer and cleaner
    checkpoint_mgr_.Start();

    is_started_ = true;
}

void Instance::Stop() {
    if (is_started_) {
        ioloop_.Stop();
        checkpoint_mgr_.Stop();
        learner_.Stop();
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
        uint64_t llMinChosenInstanceID = checkpoint_mgr_.GetMinChosenInstanceId();
        if (checkpoint_mgr_.GetMinChosenInstanceId() != llCPInstanceID)
        {
            int ret = checkpoint_mgr_.SetMinChosenInstanceID(llCPInstanceID);
            if (ret != 0)
            {
                //PLGErr("SetMinChosenInstanceID fail, now minchosen %lu max instanceid %lu checkpoint instanceid %lu",
                //        checkpoint_mgr_.GetMinChosenInstanceId(), llLogMaxInstanceID, llCPInstanceID);
                return -1;
            }

            //PLGStatus("Fix minchonse instanceid ok, old minchosen %lu now minchosen %lu max %lu checkpoint %lu",
            //        llMinChosenInstanceID, checkpoint_mgr_.GetMinChosenInstanceId(),
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

int Instance::InitLastChecksum() {
    if (acceptor_.GetInstanceId() == 0)
    {
        last_checksum_ = 0;
        return 0;
    }

    if (acceptor_.GetInstanceId() <= checkpoint_mgr_.GetMinChosenInstanceId())
    {
        last_checksum_ = 0;
        return 0;
    }

    AcceptorStateData oState;
    int ret = paxos_log_.ReadState(config_->GetMyGroupIdx(), acceptor_.GetInstanceId() - 1, oState);
    if (ret != 0 && ret != 1)
    {
        return ret;
    }

    if (ret == 1)
    {
        //PLGErr("las checksum not exist, now instanceid %lu", acceptor_.GetInstanceId());
        last_checksum_ = 0;
        return 0;
    }

    last_checksum_ = oState.checksum();

    //PLGImp("ok, last checksum %u", last_checksum_);

    return 0;
}

int Instance::PlayLog(const uint64_t llBeginInstanceID, const uint64_t llEndInstanceID) {
    if (llBeginInstanceID < checkpoint_mgr_.GetMinChosenInstanceId()) {
        //PLGErr("now instanceid %lu small than min chosen instanceid %lu",
        //        llBeginInstanceID, checkpoint_mgr_.GetMinChosenInstanceId());
        return -2;
    }

    for (uint64_t llInstanceID = llBeginInstanceID; llInstanceID < llEndInstanceID; llInstanceID++) {
        AcceptorStateData oState;
        int ret = paxos_log_.ReadState(config_->GetMyGroupIdx(), llInstanceID, oState);
        if (ret != 0) {
            //PLGErr("log read fail, instanceid %lu ret %d", llInstanceID, ret);
            return ret;
        }

        bool bExecuteRet = state_machine_fac_.Execute(config_->GetMyGroupIdx(), llInstanceID, oState.acceptedvalue(), nullptr);
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
    return last_checksum_;
}

Committer* Instance::GetCommitter()
{
    return &commiter_;
}

Cleaner* Instance::GetCheckpointCleaner()
{
    return checkpoint_mgr_.GetCleaner();
}

Replayer* Instance::GetCheckpointReplayer()
{
    return checkpoint_mgr_.GetReplayer();
}

////////////////////////////////////////////////

void Instance::CheckNewValue()
{
    /*
    if (!learner_.IsIMLatest())
    {
        return;
    }
   */

    if (config_->IsIMFollower())
    {
        //PLGErr("I'm follower, skip this new value");
        commit_ctx_.SetResultOnlyRet(PaxosTryCommitRet_Follower_Cannot_Commit);
        return;
    }

    if (!config_->CheckConfig())
    {
        //PLGErr("I'm not in membership, skip this new value");
        commit_ctx_.SetResultOnlyRet(PaxosTryCommitRet_Im_Not_In_Membership);
        return;
    }

    if ((int)commit_ctx_.GetCommitValue().size() > MAX_VALUE_SIZE)
    {
        //PLGErr("value size %zu to large, skip this new value",
        //    commit_ctx_.GetCommitValue().size());
        commit_ctx_.SetResultOnlyRet(PaxosTryCommitRet_Value_Size_TooLarge);
        return;
    }

    commit_ctx_.StartCommit(proposer_.GetInstanceId());

    if (commit_ctx_.GetTimeoutMs() != -1) {
        ioloop_.AddTimer(commit_ctx_.GetTimeoutMs(), Timer_Instance_Commit_Timeout, commit_timer_id_);
    }

    time_stat_.Point();

    if (config_->GetIsUseMembership() && (proposer_.GetInstanceId() == 0 || config_->GetGid() == 0)) {
        //Init system variables.
        //PLGHead("Need to init system variables, Now.InstanceID %lu Now.Gid %lu",
        //        proposer_.GetInstanceId(), config_->GetGid());

        uint64_t llGid = Util::GenGid(config_->GetMyNodeID());
        std::string sValue;
        int ret = config_->GetSystemVSM()->CreateGid_OPValue(llGid, sValue);
        assert(ret == 0);

        state_machine_fac_.PackPaxosValue(sValue, config_->GetSystemVSM()->StateMachineId());
        //proposer_.Propose(sValue);
    }
    else {
        if (options_.open_change_value_before_propose_) {
            state_machine_fac_.BeforePropose(config_->GetMyGroupIdx(), commit_ctx_.GetCommitValue());
        }
        //proposer_.Propose(commit_ctx_.GetCommitValue());
    }
}

void Instance::OnNewValueCommitTimeout() {
    proposer_.ExitPrepare();
    proposer_.ExitAccept();

    commit_ctx_.SetResult(PaxosTryCommitRet_Timeout, proposer_.GetInstanceId(), "");
}

int Instance::OnReceiveMessage(const char* pcMessage, const int iMessageLen)
{
    ioloop_.AddMessage(pcMessage, iMessageLen);

    return 0;
}

bool Instance::ReceiveMsgHeaderCheck(const Header & oHeader, const uint64_t iFromNodeID) {
    if (config_->GetGid() == 0 || oHeader.gid() == 0) {
        return true;
    }

    if (config_->GetGid() != oHeader.gid()) {
        return false;
    }

    return true;
}

void Instance::OnReceive(const std::string & sBuffer) {
    if (sBuffer.size() <= 6) {
        return;
    }

    Header oHeader;
    size_t iBodyStartPos = 0;
    size_t iBodyLen = 0;
    int ret = Base::UnPackBaseMsg(sBuffer, oHeader, iBodyStartPos, iBodyLen);
    if (ret != 0) {
        return;
    }

    int iCmd = oHeader.cmdid();
    if (iCmd == MsgCmd_PaxosMsg) {
        if (checkpoint_mgr_.InAskforcheckpointMode()) {
            return;
        }

        PaxosMsg oPaxosMsg;
        bool is_succeeded = oPaxosMsg.ParseFromArray(sBuffer.data() + iBodyStartPos, iBodyLen);
        if (!is_succeeded) {
            return;
        }

        if (!ReceiveMsgHeaderCheck(oHeader, oPaxosMsg.nodeid())) {
            return;
        }
        OnReceivePaxosMsg(oPaxosMsg);
    }
    else if (iCmd == MsgCmd_CheckpointMsg) {
        CheckpointMsg oCheckpointMsg;
        bool is_succeeded = oCheckpointMsg.ParseFromArray(sBuffer.data() + iBodyStartPos, iBodyLen);
        if (!is_succeeded) {
            return;
        }

        if (!ReceiveMsgHeaderCheck(oHeader, oCheckpointMsg.nodeid())) {
            return;
        }
        OnReceiveCheckpointMsg(oCheckpointMsg);
    }
}

void Instance::OnReceiveCheckpointMsg(const CheckpointMsg & oCheckpointMsg) {
    if (oCheckpointMsg.msgtype() == CheckpointMsgType_SendFile) {
        if (!checkpoint_mgr_.InAskforcheckpointMode()) {
            return;
        }

        //learner_.OnSendCheckpoint(oCheckpointMsg);
    }
    else if (oCheckpointMsg.msgtype() == CheckpointMsgType_SendFile_Ack)
    {
        //learner_.OnSendCheckpointAck(oCheckpointMsg);
    }
}

int Instance::OnReceivePaxosMsg(const PaxosMsg & oPaxosMsg, const bool bIsRetry) {
    std::vector<PaxosMsgType> prepare_related_msgs{
        MsgType_PaxosPrepareReply,
        MsgType_PaxosAcceptReply,
        MsgType_PaxosProposal_SendNewValue
    };
    std::vector<PaxosMsgType> normal_msgs{
        MsgType_PaxosAccept,
        MsgType_PaxosPrepare
    };
    std::vector<PaxosMsgType> learner_related_msgs{
        MsgType_PaxosLearner_AskforLearn,
        MsgType_PaxosLearner_SendLearnValue,
        MsgType_PaxosLearner_ProposerSendSuccess,
        MsgType_PaxosLearner_ComfirmAskforLearn,
        MsgType_PaxosLearner_SendNowInstanceID,
        MsgType_PaxosLearner_SendLearnValue_Ack,
        MsgType_PaxosLearner_AskforCheckpoint
    };

    auto msg_type = oPaxosMsg.msgtype();
    if (std::find(prepare_related_msgs.begin(), prepare_related_msgs.end(), msg_type) != prepare_related_msgs.end()) {
        if (!config_->IsValidNodeID(oPaxosMsg.nodeid())) {
            ////BP->GetInstance//BP()->OnReceivePaxosMsgNodeIDNotValid();
            //PLGErr("acceptor reply type msg, from nodeid not in my membership, skip this message");
            return 0;
        }
        return ReceiveMsgForProposer(oPaxosMsg);
    }
    else if (std::find(normal_msgs.begin(), normal_msgs.end(), msg_type) != normal_msgs.end()) {
        if (config_->GetGid() == 0) {
            config_->AddTmpNodeOnlyForLearn(oPaxosMsg.nodeid());
        }

        if ((!config_->IsValidNodeID(oPaxosMsg.nodeid()))) {
            config_->AddTmpNodeOnlyForLearn(oPaxosMsg.nodeid());

            return 0;
        }

        ChecksumLogic(oPaxosMsg);
        return ReceiveMsgForAcceptor(oPaxosMsg, bIsRetry);
    }
    else if (std::find(learner_related_msgs.begin(), learner_related_msgs.end(), msg_type) != learner_related_msgs.end()) {
        ChecksumLogic(oPaxosMsg);
        return ReceiveMsgForLearner(oPaxosMsg);
    }
    else {
    }

    return 0;
}

int Instance::ReceiveMsgForProposer(const PaxosMsg & oPaxosMsg) {
    if (config_->IsIMFollower()) {
        //PLGErr("I'm follower, skip this message");
        return 0;
    }

    if (oPaxosMsg.instanceid() != proposer_.GetInstanceId()) {
        if (oPaxosMsg.instanceid() + 1 == proposer_.GetInstanceId()) {
            //Exipred reply msg on last instance.
            //If the response of a node is always slower than the majority node,
            //then the message of the node is always ignored even if it is a reject reply.
            //In this case, if we do not deal with these reject reply, the node that
            //gave reject reply will always give reject reply.
            //This causes the node to remain in catch-up state.
            //
            //To avoid this problem, we need to deal with the expired reply.
            if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply) {
                //proposer_.OnExpiredPrepareReply(oPaxosMsg);
            }
            else if (oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply) {
                //proposer_.OnExpiredAcceptReply(oPaxosMsg);
            }
        }

        ////BP->GetInstance//BP()->OnReceivePaxosProposerMsgInotsame();
        //PLGErr("InstanceID not same, skip msg");
        return 0;
    }

    if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply) {
        //proposer_.OnPrepareReply(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply) {
        //proposer_.OnAcceptReply(oPaxosMsg);
    }

    return 0;
}

int Instance::ReceiveMsgForAcceptor(const PaxosMsg & oPaxosMsg, const bool bIsRetry) {
    if (config_->IsIMFollower()) {
        //PLGErr("I'm follower, skip this message");
        return 0;
    }

    if (oPaxosMsg.instanceid() != acceptor_.GetInstanceId()) {
        ////BP->GetInstance//BP()->OnReceivePaxosAcceptorMsgInotsame();
    }

    if (oPaxosMsg.instanceid() == acceptor_.GetInstanceId() + 1) {
        //skip success message
        PaxosMsg oNewPaxosMsg = oPaxosMsg;
        oNewPaxosMsg.set_instanceid(acceptor_.GetInstanceId());
        oNewPaxosMsg.set_msgtype(MsgType_PaxosLearner_ProposerSendSuccess);

        ReceiveMsgForLearner(oNewPaxosMsg);
    }

    if (oPaxosMsg.instanceid() == acceptor_.GetInstanceId()) {
        if (oPaxosMsg.msgtype() == MsgType_PaxosPrepare) {
            return acceptor_.OnPrepare(oPaxosMsg);
        }
        else if (oPaxosMsg.msgtype() == MsgType_PaxosAccept) {
            acceptor_.OnAccept(oPaxosMsg);
        }
    }
    else if ((!bIsRetry) && (oPaxosMsg.instanceid() > acceptor_.GetInstanceId())) {
        //retry msg can't retry again.
        if (oPaxosMsg.instanceid() >= learner_.GetLatestInstanceID()) {
            if (oPaxosMsg.instanceid() < acceptor_.GetInstanceId() + RETRY_QUEUE_MAX_LEN) {
                //need retry msg precondition
                //1. prepare or accept msg
                //2. msg.instanceid > nowinstanceid.
                //    (if < nowinstanceid, this msg is expire)
                //3. msg.instanceid >= seen latestinstanceid.
                //    (if < seen latestinstanceid, proposer don't need reply with this instanceid anymore.)
                //4. msg.instanceid close to nowinstanceid.
                ioloop_.AddRetryPaxosMsg(oPaxosMsg);
            }
            else {
                //retry msg not series, no use.
                ioloop_.ClearRetryQueue();
            }
        }
    }

    return 0;
}

int Instance::ReceiveMsgForLearner(const PaxosMsg & oPaxosMsg)
{
    if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforLearn)
    {
        //learner_.OnAskforLearn(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue)
    {
        //learner_.OnSendLearnValue(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_ProposerSendSuccess)
    {
        //learner_.OnProposerSendSuccess(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendNowInstanceID)
    {
        //learner_.OnSendNowInstanceID(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_ComfirmAskforLearn)
    {
        //learner_.OnComfirmAskForLearn(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_SendLearnValue_Ack)
    {
        //learner_.OnSendLearnValue_Ack(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosLearner_AskforCheckpoint)
    {
        //learner_.OnAskforCheckpoint(oPaxosMsg);
    }

    return 0;
}

void Instance::NewInstance() {
    acceptor_.NewInstance();
    learner_.NewInstance();
    proposer_.NewInstance();
}

const uint64_t Instance::GetInstanceId() {
    return acceptor_.GetInstanceId();
}

const uint64_t Instance::GetMinChosenInstanceId() {
    //return checkpoint_mgr_.GetMinChosenInstanceId();
}

void Instance::OnTimeout(const uint32_t iTimerID, const int iType)
{
    if (iType == Timer_Proposer_Prepare_Timeout)
    {
        proposer_.OnPrepareTimeout();
    }
    else if (iType == Timer_Proposer_Accept_Timeout)
    {
        proposer_.OnAcceptTimeout();
    }
    else if (iType == Timer_Learner_Askforlearn_noop)
    {
        //learner_.AskforLearn_Noop();
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

void Instance::AddStateMachine(StateMachine* poSM) {
}

bool Instance::SMExecute(
        const uint64_t llInstanceID,
        const std::string & sValue,
        const bool bIsMyCommit,
        StateMachineCtx* poStateMachineCtx
        ) {
    return state_machine_fac_.Execute(config_->GetMyGroupIdx(), llInstanceID, sValue, poStateMachineCtx);
}

void Instance::ChecksumLogic(const PaxosMsg & oPaxosMsg) {
    if (oPaxosMsg.instanceid() != acceptor_.GetInstanceId()) {
        return;
    }

    if (acceptor_.GetInstanceId() > 0 && GetLastChecksum() == 0) {
        //PLGErr("I have no last checksum, other last checksum %u", oPaxosMsg.lastchecksum());
        last_checksum_ = oPaxosMsg.lastchecksum();
        return;
    }
    assert(oPaxosMsg.lastchecksum() == GetLastChecksum());
}

int Instance::GetInstanceValue(const uint64_t llInstanceID, std::string & sValue, int & iSMID) {
    iSMID = 0;

    if (llInstanceID >= acceptor_.GetInstanceId())
    {
        return Paxos_GetInstanceValue_Value_Not_Chosen_Yet;
    }

    AcceptorStateData oState;
    int ret = paxos_log_.ReadState(config_->GetMyGroupIdx(), llInstanceID, oState);
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

#pragma once

namespace paxos {

#define SYSTEM_V_STATE_MACHINE_ID 100000000
#define MASTER_V_STATE_MACHINE_ID 100000001
#define BATCH_PROPOSE_STATE_MACHINE_ID 100000002

#define MAX_QUEUE_MEM_SIZE 209715200

#define CRC32SKIP 8
#define NET_CRC32SKIP 7 

enum PaxosTryCommitRet {
    PaxosTryCommitRet_OK = 0,
    PaxosTryCommitRet_Reject = -2,
    PaxosTryCommitRet_Conflict = 14,
    PaxosTryCommitRet_ExecuteFail = 15,
    PaxosTryCommitRet_Follower_Cannot_Commit = 16,
    PaxosTryCommitRet_Im_Not_In_Membership  = 17,
    PaxosTryCommitRet_Value_Size_TooLarge = 18,
    PaxosTryCommitRet_Timeout = 404,
    PaxosTryCommitRet_TooManyThreadWaiting_Reject = 405,
};

enum PaxosNodeFunctionRet {
    Paxos_SystemError = -1,
    Paxos_GroupIdxWrong = -5,
    Paxos_MembershipOp_GidNotSame = -501,
    Paxos_MembershipOp_VersionConflit = -502,
    Paxos_MembershipOp_NoGid = 1001,
    Paxos_MembershipOp_Add_NodeExist = 1002,
    Paxos_MembershipOp_Remove_NodeNotExist = 1003,
    Paxos_MembershipOp_Change_NoChange = 1004,
    Paxos_GetInstanceValue_Value_NotExist = 1005,
    Paxos_GetInstanceValue_Value_Not_Chosen_Yet = 1006,
};


enum MsgCmd {
    MsgCmd_PaxosMsg = 1,
    MsgCmd_CheckpointMsg = 2,
};

enum PaxosMsgType {
    MsgType_PaxosPrepare = 1,
    MsgType_PaxosPrepareReply = 2,
    MsgType_PaxosAccept = 3,
    MsgType_PaxosAcceptReply = 4,
    MsgType_PaxosLearner_AskforLearn = 5,
    MsgType_PaxosLearner_SendLearnValue = 6,
    MsgType_PaxosLearner_ProposerSendSuccess = 7,
    MsgType_PaxosProposal_SendNewValue = 8,
    MsgType_PaxosLearner_SendNowInstanceID = 9,
    MsgType_PaxosLearner_ComfirmAskforLearn = 10,
    MsgType_PaxosLearner_SendLearnValue_Ack = 11,
    MsgType_PaxosLearner_AskforCheckpoint = 12,
    MsgType_PaxosLearner_OnAskforCheckpoint = 13,
};

enum PaxosMsgFlagType {
    PaxosMsgFlagType_SendLearnValue_NeedAck = 1,
};

enum CheckpointMsgType {
    CheckpointMsgType_SendFile = 1,
    CheckpointMsgType_SendFile_Ack = 2,
};

enum CheckpointSendFileFlag {
    CheckpointSendFileFlag_BEGIN = 1,
    CheckpointSendFileFlag_ING = 2,
    CheckpointSendFileFlag_END = 3,
};

enum CheckpointSendFileAckFlag {
    CheckpointSendFileAckFlag_OK = 1,
    CheckpointSendFileAckFlag_Fail = 2,
};

enum TimerType {
    Timer_Proposer_Prepare_Timeout = 1,
    Timer_Proposer_Accept_Timeout = 2,
    Timer_Learner_Askforlearn_noop = 3,
    Timer_Instance_Commit_Timeout = 4,
};

}
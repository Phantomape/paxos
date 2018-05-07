#pragma once

#include <inttypes.h>
#include <typeinfo>

namespace paxos {

#define MAX_VALUE_SIZE (InternalOptions::Instance()->GetMaxBufferSize())
#define START_PREPARE_TIMEOUTMS (InternalOptions::Instance()->GetStartPrepareTimeoutMs())
#define START_ACCEPT_TIMEOUTMS (InternalOptions::Instance()->GetStartAcceptTimeoutMs())
#define MAX_PREPARE_TIMEOUTMS (InternalOptions::Instance()->GetMaxPrepareTimeoutMs())
#define MAX_ACCEPT_TIMEOUTMS (InternalOptions::Instance()->GetMaxAcceptTimeoutMs())
#define QUEUE_MAXLENGTH (InternalOptions::Instance()->GetMaxIOLoopQueueLen())
#define ASKFORLEARN_NOOP_INTERVAL (InternalOptions::Instance()->GetAskforLearnInterval())
#define LEARNER_SYNCHRONIZER_PREPARE_TIMEOUT (InternalOptions::Instance()->GetLearnerSynchronizerPrepareTimeoutMs())
#define LEARNER_SYNCHRONIZER_ACK_TIMEOUT (InternalOptions::Instance()->GetLearnerSynchronizer_Ack_TimeoutMs())
#define LEARNER_SYNCHRONIZER_ACK_LEAD (InternalOptions::Instance()->GetLearnerSynchronizer_Ack_Lead())
#define LEARNER_RECEIVER_ACK_LEAD (InternalOptions::Instance()->GetLearnerReceiver_Ack_Lead())
#define TCP_QUEUE_MAXLEN (InternalOptions::Instance()->GetMaxQueueLen())
#define UDP_QUEUE_MAXLEN (InternalOptions::Instance()->GetMaxQueueLen())
#define TCP_OUTQUEUE_DROP_TIMEMS (InternalOptions::Instance()->GetTcpOutQueueDropTimeMs())
#define LOG_FILE_MAX_SIZE (InternalOptions::Instance()->GetLogFileMaxSize())
#define CONNECTTION_NONACTIVE_TIMEOUT (InternalOptions::Instance()->GetTcpConnectionNonActiveTimeout())
#define LEARNER_SYNCHRONIZER_SEND_QPS (InternalOptions::Instance()->GetLearnerSynchronizerSendQps())
#define Cleaner_DELETE_QPS (InternalOptions::Instance()->GetCleanerDeleteQps())

class InternalOptions
{
public:
    InternalOptions();
    ~InternalOptions();

    static InternalOptions * Instance();

    void SetAsLargeBufferMode();

    void SetAsFollower();

    void SetGroupCount(const int iGroupCount);

public:
    const int GetMaxBufferSize();

    const int GetStartPrepareTimeoutMs();

    const int GetStartAcceptTimeoutMs();

    const int GetMaxPrepareTimeoutMs();

    const int GetMaxAcceptTimeoutMs();

    const int GetMaxIOLoopQueueLen();

    const int GetMaxQueueLen();

    const int GetAskforLearnInterval();

    const int GetLearnerReceiver_Ack_Lead();

    const int GetLearnerSynchronizerPrepareTimeoutMs();

    const int GetLearnerSynchronizer_Ack_TimeoutMs();

    const int GetLearnerSynchronizer_Ack_Lead();

    const int GetTcpOutQueueDropTimeMs();

    const int GetLogFileMaxSize();

    const int GetTcpConnectionNonActiveTimeout();

    const int GetLearnerSynchronizerSendQps();

    const int GetCleanerDeleteQps();

private:
    bool is_large_buffer_mode_;
    bool is_im_follower_;
    int group_count_;
};
    
}

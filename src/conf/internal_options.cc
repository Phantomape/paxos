#include "internal_options.h"
#include "util.h"

namespace paxos {

InternalOptions::InternalOptions() {
    is_large_buffer_mode_ = false;
    is_im_follower_ = false;
    group_count_ = 1;
}

InternalOptions::~InternalOptions() {
}

InternalOptions * InternalOptions::Instance() {
    static InternalOptions oInternalOptions;
    return &oInternalOptions;
}

void InternalOptions::SetAsLargeBufferMode() {
    is_large_buffer_mode_ = true;
}

void InternalOptions::SetAsFollower() {
    is_im_follower_ = true;
}

void InternalOptions::SetGroupCount(const int iGroupCount) {
    group_count_ = iGroupCount;
}

const int InternalOptions::GetMaxBufferSize() {
    if (is_large_buffer_mode_) {
        return 52428800;
    }
    else {
        return 10485760;
    }
}

const int InternalOptions::GetStartPrepareTimeoutMs() {
    if (is_large_buffer_mode_) {
        return 15000;
    }
    else {
        return 2000;
    }
}

const int InternalOptions::GetStartAcceptTimeoutMs() {
    if (is_large_buffer_mode_) {
        return 15000;
    }
    else {
        return 1000;
    }
}

const int InternalOptions::GetMaxPrepareTimeoutMs(){
    if (is_large_buffer_mode_) {
        return 90000;
    }
    else {
        return 8000;
    }
}

const int InternalOptions::GetMaxAcceptTimeoutMs() {
    if (is_large_buffer_mode_) {
        return 90000;
    }
    else {
        return 8000;
    }
}

const int InternalOptions::GetMaxIOLoopQueueLen() {
    if (is_large_buffer_mode_) {
        return 1024 / group_count_ + 100;
    }
    else {
        return 10240 / group_count_ + 1000;
    }
}

const int InternalOptions::GetMaxQueueLen() {
    if (is_large_buffer_mode_) {
        return 1024;
    }
    else {
        return 10240;
    }
}

const int InternalOptions::GetAskforLearnInterval() {
    if (!is_im_follower_) {
        if (is_large_buffer_mode_)
        {
            return 50000 + (Util::FastRand() % 10000);
        }
        else
        {
            return 2500 + (Util::FastRand() % 500);
        }
    }
    else
    {
        if (is_large_buffer_mode_)
        {
            return 30000 + (Util::FastRand() % 15000);
        }
        else
        {
            return 2000 + (Util::FastRand() % 1000);
        }
    }
}

const int InternalOptions::GetLearnerReceiver_Ack_Lead()
{
    if (is_large_buffer_mode_)
    {
        return 2;
    }
    else
    {
        return 4;
    }
}

const int InternalOptions::GetLearnerSynchronizerPrepareTimeoutMs() {
    if (is_large_buffer_mode_) {
        return 6000;
    }
    else {
        return 5000;
    }
}

const int InternalOptions::GetLearnerSynchronizer_Ack_TimeoutMs() {
    if (is_large_buffer_mode_) {
        return 60000;
    }
    else {
        return 5000;
    }
}

const int InternalOptions::GetLearnerSynchronizer_Ack_Lead() {
    if (is_large_buffer_mode_) {
        return 5;
    }
    else {
        return 21;
    }
}

const int InternalOptions::GetTcpOutQueueDropTimeMs() {
    if (is_large_buffer_mode_) {
        return 20000;
    }
    else {
        return 5000;
    }
}

const int InternalOptions::GetLogFileMaxSize() {
    if (is_large_buffer_mode_) {
        return 524288000;
    }
    else {
        return 104857600;
    }
}

const int InternalOptions::GetTcpConnectionNonActiveTimeout() {
    if (is_large_buffer_mode_) {
        return 600000;
    }
    else {
        return 60000;
    }
}

const int InternalOptions::GetLearnerSynchronizerSendQps() {
    if (is_large_buffer_mode_) {
        return 10000 / group_count_;
    }
    else {
        return 100000 / group_count_;
    }
}

const int InternalOptions::GetCleanerDeleteQps() {
    if (is_large_buffer_mode_) {
        return 30000 / group_count_;
    }
    else {
        return 300000 / group_count_;
    }
}

}



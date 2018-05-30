#include "config.h"
#include "def.h"
#include "instance.h"

namespace paxos {

IoLoop::IoLoop(Config* config, Instance* instance)
    : config_(config), instance_(instance) {
    is_ended_ = false;
    is_started_ = false;
    queue_mem_size_ = 0;
}

IoLoop::~IoLoop() {
}

void IoLoop::Run() {
    is_ended_ = false;
    is_started_ = true;
    while(true) {
        //BP->GetIoLoopBP()->OneLoop();

        int iNextTimeout = 1000;
        DealwithTimeout(iNextTimeout);
        Loop(iNextTimeout);

        if (is_ended_) {
            break;
        }
    }
}

void IoLoop::AddNotify() {
    message_queue_.lock();
    message_queue_.add(nullptr);
    message_queue_.unlock();
}

int IoLoop::AddMessage(const char * pcMessage, const int iMessageLen) {
    message_queue_.lock();

    //BP->GetIoLoopBP()->EnqueueMsg();

    if ((int)message_queue_.size() > QUEUE_MAXLENGTH) {
        //BP->GetIoLoopBP()->EnqueueMsgRejectByFullQueue();

        //PLGErr("Queue full, skip msg");
        message_queue_.unlock();
        return -2;
    }

    if (queue_mem_size_ > MAX_QUEUE_MEM_SIZE) {
        //PLErr("queue memsize %d too large, can't enqueue", queue_mem_size_);
        message_queue_.unlock();
        return -2;
    }
    
    message_queue_.add(new std::string(pcMessage, iMessageLen));

    queue_mem_size_ += iMessageLen;

    message_queue_.unlock();

    return 0;
}

int IoLoop::AddRetryPaxosMsg(const PaxosMsg & paxos_msg) {
    //BP->GetIoLoopBP()->EnqueueRetryMsg();

    if (retry_queue_.size() > RETRY_QUEUE_MAX_LEN) {
        //BP->GetIoLoopBP()->EnqueueRetryMsgRejectByFullQueue();
        retry_queue_.pop();
    }
    
    retry_queue_.push(paxos_msg);
    return 0;
}

void IoLoop::Stop() {
    is_ended_ = true;
    if (is_started_) {
        Join();
    }
}

void IoLoop::ClearRetryQueue() {
    while (!retry_queue_.empty()) {
        retry_queue_.pop();
    }
}

void IoLoop::DealWithRetry() {
    if (retry_queue_.empty()) {
        return;
    }
    
    bool bHaveRetryOne = false;
    while (!retry_queue_.empty()) {
        PaxosMsg & paxos_msg = retry_queue_.front();
        if (paxos_msg.instanceid() > instance_->GetInstanceId() + 1) {
            break;
        }
        else if (paxos_msg.instanceid() == instance_->GetInstanceId() + 1) {
            //only after retry i == now_i, than we can retry i + 1.
            if (bHaveRetryOne) {
                //BP->GetIoLoopBP()->DealWithRetryMsg();
                //PLGDebug("retry msg (i+1). instanceid %lu", paxos_msg.instanceid());
                instance_->OnReceivePaxosMsg(paxos_msg, true);
            }
            else {
                break;
            }
        }
        else if (paxos_msg.instanceid() == instance_->GetInstanceId()) {
            //BP->GetIoLoopBP()->DealWithRetryMsg();
            //PLGDebug("retry msg. instanceid %lu", paxos_msg.instanceid());
            instance_->OnReceivePaxosMsg(paxos_msg);
            bHaveRetryOne = true;
        }

        retry_queue_.pop();
    }
}

void IoLoop::Loop(const int timeout_ms) {
    std::string* msg = nullptr;

    message_queue_.lock();
    bool is_succeeded = message_queue_.peek(msg, timeout_ms);
    
    if (!is_succeeded) {
        message_queue_.unlock();
    }
    else {
        message_queue_.pop();
        message_queue_.unlock();

        // Fuck, how can you get into here!!!
        if (msg != nullptr && msg->size() > 0) {
            queue_mem_size_ -= msg->size();
            instance_->OnReceive(*msg);
        }

        delete msg;
    }

    DealWithRetry();

    //must put on here
    //because addtimer on this funciton
    instance_->CheckNewValue();
}

bool IoLoop::AddTimer(const int iTimeout, const int iType, uint32_t & iTimerID) {
    if (iTimeout == -1) {
        return true;
    }
    
    uint64_t llAbsTime = Time::GetSteadyClockMS() + iTimeout;
    timer_.AddTimerWithType(llAbsTime, iType, iTimerID);

    map_timer_id_exist_[iTimerID] = true;

    return true;
}

void IoLoop::RemoveTimer(uint32_t & iTimerID) {
    auto it = map_timer_id_exist_.find(iTimerID);
    if (it != end(map_timer_id_exist_)) {
        map_timer_id_exist_.erase(it);
    }

    iTimerID = 0;
}

void IoLoop::DealwithTimeoutOne(const uint32_t iTimerID, const int iType) {
    auto it = map_timer_id_exist_.find(iTimerID);
    if (it == end(map_timer_id_exist_)) {
        //PLGErr("Timeout aready remove!, timerid %u iType %d", iTimerID, iType);
        return;
    }

    map_timer_id_exist_.erase(it);

    instance_->OnTimeout(iTimerID, iType);
}

void IoLoop::DealwithTimeout(int & iNextTimeout) {
    bool bHasTimeout = true;

    while(bHasTimeout) {
        uint32_t iTimerID = 0;
        int iType = 0;
        bHasTimeout = timer_.PopTimeout(iTimerID, iType);

        if (bHasTimeout) {
            DealwithTimeoutOne(iTimerID, iType);

            iNextTimeout = timer_.GetNextTimeout();
            if (iNextTimeout != 0) {
                break;
            }
        }
    }
}

}
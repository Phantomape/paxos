#include "config.h"
#include "ioloop.h"

namespace paxos {

IoLoop::IoLoop(Config* config, Instance* instance)
    : config_(config), instance_(instance) {
    is_ended_ = false;
    is_started_ = false;

    queue_size_ = 0;
}

IoLoop::~IoLoop() {}

void IoLoop::AddNotify() {
    message_queue_.lock();
    message_queue_.add(nullptr);
    message_queue_.unlock();
}

int IoLoop::AddRetryPaxosMsg(const PaxosMsg& paxos_msg) {
    if (retry_queue_.size() > RETRY_QUEUE_MAX_LEN) {
        retry_queue_.pop();
    }

    retry_queue_.push(paxos_msg);
    return 0;
}

void IoLoop::ClearRetryQueue() {
    while (!retry_queue_.empty()) {
        retry_queue_.pop();
    }
}

void IoLoop::Run() {
    is_ended_ = false;
    is_started_ = true;
    while (true) {
        int next_timeout = 1000;
        // Deal with time out
        Init(next_timeout);
        if (is_ended_) {
            break;
        }
    }    
}

void IoLoop::Init(const int timeout_ms) {
    std::string* message = nullptr;
    
    message_queue_.lock();
    bool is_succeeded = message_queue_.peek(message, timeout_ms);
    if (!is_succeeded) {
        message_queue_.unlock();
    }
    else {
        message_queue_.pop();
        message_queue_.unlock();

        if (message != nullptr && message->size() > 0) {
            queue_size_ -= message->size();
            instance_->OnReceive(*message);
        }
        // Why not delete outside the if section
        delete message;
    }

    DealWithRetry();

    instance_->CheckNewValue();
}

void IoLoop::DealWithRetry() {
    if (retry_queue_.empty()) {
        return;
    }

    bool has_retried = false;
    while (!retry_queue_.empty()) {
        PaxosMsg& paxos_msg = retry_queue_.front();
        if (paxos_msg.instanceid() > instance_->GetCurrentInstanceId() + 1) {
            break;
        }
        else if (paxos_msg.instanceid() == instance_->GetCurrentInstanceId() + 1) {
            if (has_retried) {
                instance_->OnReceivePaxosMsg(paxos_msg, true);
            }
            else {
                break;
            }
        }
        else if (paxos_msg.instanceid() == instance_->GetCurrentInstanceId()) {
            instance_->OnReceivePaxosMsg(paxos_msg);
            has_retried = true;
        }

        retry_queue_.pop();
    }
}

}
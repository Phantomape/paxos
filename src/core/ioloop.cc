#include "ioloop.h"

namespace paxos {

IoLoop::IoLoop(Instance* instance) : instance(instance) {}

IoLoop::~IoLoop() {}

void IoLoop::AddNotify() {
    message_queue.lock();
    message_queue.add(nullptr);
    message_queue.unlock();
}

int IoLoop::AddRetryPaxosMsg(const PaxosMsg& paxos_msg) {
    if (retry_queue.size() > RETRY_QUEUE_MAX_LEN) {
        retry_queue.pop();
    }

    retry_queue.push(paxos_msg);
    return 0;
}

void IoLoop::ClearRetryQueue() {
    while (!retry_queue.empty()) {
        retry_queue.pop();
    }
}

void IoLoop::Run() {
    is_end_ = false;
    is_start_ = true;
    while (true) {
        int next_timeout = 1000;
        // Deal with time out
        Init(next_timeout);
        if (is_end_) {
            break;
        }
    }    
}

void IoLoop::Init(const int timeout_ms) {
    std::string* message = nullptr;
    
    message_queue.lock();
    bool is_succeeded = message_queue.peek(message, timeout_ms);
    if (!is_succeeded) {
        message_queue.unlock();
    }
    else {
        message_queue.pop();
        message_queue.unlock();

        if (message != nullptr && message->size() > 0) {
            queue_size_ -= message->size();
            instance->OnReceive(*message);
        }
        // Why not delete outside the if section
        delete message;
    }

    DealWithRetry();

    instance->CheckNewValue();
}

void IoLoop::DealWithRetry() {
    if (retry_queue.empty()) {
        return;
    }

    bool has_retried = false;
    while (!retry_queue.empty()) {
        PaxosMsg& paxos_msg = retry_queue.front();
        if (paxos_msg.instanceid() > instance->GetCurrentInstanceId() + 1) {
            break;
        }
        else if (paxos_msg.instanceid() == instance->GetCurrentInstanceId() + 1) {
            if (has_retried) {
                instance->OnReceivePaxosMsg(paxos_msg, true);
            }
            else {
                break;
            }
        }
        else if (paxos_msg.instanceid() == instance->GetCurrentInstanceId()) {
            instance->OnReceivePaxosMsg(paxos_msg);
            has_retried = true;
        }

        retry_queue.pop();
    }
}

}
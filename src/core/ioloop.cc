#include "ioloop.h"

namespace paxos {

IoLoop::IoLoop() {}

IoLoop::~IoLoop() {}

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
    
}

}
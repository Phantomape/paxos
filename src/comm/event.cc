#include "event.h"
#include "event_loop.h"

namespace paxos {

Event::Event(EventLoop * event_loop) : event_(0), event_loop(event_loop) {
    is_destroried_ = false;
}

Event::~Event() {}

int Event::OnRead() {
    return -1;
}

int Event::OnWrite() {
    return -1;
}

void Event::OnTimeout(const uint32_t timer_id, const int type) {}

void Event::JumpoutEpollWait() {
    event_loop->JumpoutEpollWait();
}

void Event :: AddEvent(const int event) {
    int previous_event = event;
    event_ |= event; 
    if (event_ == previous_event) {
        return;
    }
    
    event_loop->ModEvent(this, event_);
}

void Event::RemoveEvent(const int event)
{
    int previous_event = event;
    event_ &= (~event);
    if (event_ == previous_event) {
        return;
    }
    
    if (event_ == 0) {
        event_loop->RemoveEvent(this);
    }
    else {
        event_loop->ModEvent(this, event);
    }
}

void Event::AddTimer(const int timeout_ms, const int type, uint32_t& timer_id) {
    event_loop->AddTimer(this, timeout_ms, type, timer_id);
}

void Event::RemoveTimer(const uint32_t timer_id) {
    event_loop->RemoveTimer(timer_id);
}

void Event::Destroy() {
    is_destroried_ = true;
}

const bool Event :: IsDestroy() const {
    return is_destroried_;
}

}
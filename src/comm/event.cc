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

/*
void Event::JumpoutEpollWait() {
    event_loop->JumpoutEpollWait();
}
*/

/*
void Event :: AddEvent(const int iEvents)
{
    int iBeforeEvent = m_iEvents;
    m_iEvents |= iEvents; 
    if (m_iEvents == iBeforeEvent)
    {
        return;
    }
    
    event_loop->ModEvent(this, m_iEvents);
}
*/

void Event::RemoveEvent(const int event)
{
    int previous_event = event;
    event_ &= (~event);
    if (event_ == previous_event) {
        return;
    }
    
    if (event_ == 0) {
        // event_loop->RemoveEvent(this);
    }
    else {
        //event_loop->ModEvent(this, m_iEvents);
    }
}

void Event::AddTimer(const int timeout_ms, const int type, uint32_t& timer_id) {
    //event_loop->AddTimer(this, iTimeoutMs, iType, iTimerID);
}

void Event::RemoveTimer(const uint32_t iTimerID) {
    //event_loop->RemoveTimer(iTimerID);
}

void Event::Destroy() {
    is_destroried_ = true;
}

const bool Event :: IsDestroy() const {
    return is_destroried_;
}

}
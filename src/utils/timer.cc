#include "timer.h"
#include "util.h"
#include <algorithm> 

namespace paxos {

Timer::Timer() : current_timer_id_(1) {
}

Timer::~Timer() {
}

void Timer::AddTimer(const uint64_t abs_time, uint32_t & timer_id) {
    return AddTimerWithType(abs_time, 0, timer_id);
}

void Timer::AddTimerWithType(const uint64_t abs_time, const int type, uint32_t & timer_id) {
    timer_id = current_timer_id_++;

    TimerObj tObj(timer_id, abs_time, type);
    vec_timer_heap_.push_back(tObj);
    push_heap(begin(vec_timer_heap_), end(vec_timer_heap_));
}

const int Timer::GetNextTimeout() const
{
    if (vec_timer_heap_.empty())
    {
        return -1;
    }

    int iNextTimeout = 0;

    TimerObj tObj = vec_timer_heap_.front();
    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (tObj.abs_time_ > llNowTime)
    {
        iNextTimeout = (int)(tObj.abs_time_ - llNowTime);
    }

    return iNextTimeout;
}

bool Timer::PopTimeout(uint32_t & timer_id, int & type)
{
    if (vec_timer_heap_.empty())
    {
        return false;
    }

    TimerObj tObj = vec_timer_heap_.front();
    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (tObj.abs_time_ > llNowTime)
    {
        return false;
    }
    
    pop_heap(begin(vec_timer_heap_), end(vec_timer_heap_));
    vec_timer_heap_.pop_back();

    timer_id = tObj.timer_id_;
    type = tObj.type_;

    return true;
}    
    
}



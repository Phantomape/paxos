#pragma once

#include <vector>
#include <inttypes.h>

namespace paxos {

class Timer {
public:
    Timer();

    ~Timer();

    void AddTimer(const uint64_t abs_time, uint32_t & timer_id);
    
    void AddTimerWithType(const uint64_t abs_time, const int type, uint32_t & timer_id);

    bool PopTimeout(uint32_t & timer_id, int & type);

    const int GetNextTimeout() const;
    
private:
    struct TimerObj
    {
        TimerObj(uint32_t timer_id, uint64_t abs_time, int type) 
            : timer_id_(timer_id), abs_time_(abs_time), type_(type) {}

        uint32_t timer_id_;
        uint64_t abs_time_;
        int type_;

        bool operator < (const TimerObj & obj) const {
            if (obj.abs_time_ == abs_time_) {
                return obj.timer_id_ < timer_id_;
            }
            else {
                return obj.abs_time_ < abs_time_;
            }
        }
    };

private:
    uint32_t current_timer_id_;
    std::vector<TimerObj> vec_timer_heap_;
};
    
}

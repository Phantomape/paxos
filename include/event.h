#pragma once

#include <string>

namespace paxos {

class EventLoop;

class Event {
public: 
    Event(EventLoop* event_loop);

    virtual ~Event();

    virtual int GetSocketFd() const = 0;

    virtual const std::string & GetSocketHost() = 0;

    virtual int OnRead();

    virtual int OnWrite();

    virtual void OnError(bool& need_delete) = 0;

    virtual void OnTimeout(const uint32_t timer_id, const int type);

    void AddEvent(const int event);

    void RemoveEvent(const int event);

    void JumpoutEpollWait();

    const bool IsDestroy() const;

    void Destroy();

    void AddTimer(const int timeout_ms, const int type, uint32_t& timer_id);

    void RemoveTimer(const uint32_t timer_id);

protected:
    int event_;
    EventLoop* event_loop;

    bool is_destroried_;
};

}
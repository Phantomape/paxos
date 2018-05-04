#pragma once

#include "event.h"
#include <string>

namespace paxos {

class EventLoop;

class NotifyEvent : public Event {
public:
    NotifyEvent(EventLoop* event_loop);
    ~NotifyEvent();

    int Init();

    int GetSocketFd() const;

    const std::string & GetSocketHost();

    void SendNotifyEvent();

    int OnRead();

    void OnError(bool & bNeedDelete);

private:
    int pipe_fd_[2];
    std::string host_;
};

}
#include "event.h"
#include "event_loop.h"
#include "message_event.h"
#include "network.h"
#include <cstring>
#include <sys/epoll.h>


namespace paxos {

EventLoop::EventLoop(Network* network) {
    epoll_fd_ = -1;
    is_ended_ = false;
    this->network = network;
    std::memset(epoll_events, 0, sizeof(epoll_events));
}

EventLoop::~EventLoop() {
    ClearEvent();
}

void EventLoop::ClearEvent() {
    
}

void EventLoop::AddEvent(int fd, SocketAddress addr)
{
    std::lock_guard<std::mutex> lck(mutex_);
    queue_socket_addr_.push(std::make_pair(fd, addr));
}

bool EventLoop::AddTimer(const Event* event, const int timeout, const int type, uint32_t& timer_id) {
    if (event->GetSocketFd() == 0) {
        return false;
    }

    if (mapping_id_2_event_context_.find(event->GetSocketFd()) == mapping_id_2_event_context_.end()) {
        EventCtx ctx;
        ctx.event = (Event*)event;
        ctx.event_ = 0;

        mapping_id_2_event_context_[event->GetSocketFd()] = ctx;
    }

    mapping_timer_id_2_fd_[timer_id] = event->GetSocketFd();

    return true;
}

void EventLoop::CreateEvent() {
    std::lock_guard<std::mutex> lck(mutex_);

    if (false) {
        return;
    }

    ClearEvent();

    int num_events_create_per_time = 200;
    // Do sth. related to the queue
}

void EventLoop::DealwithTimeout(int& timeout) {
    bool has_timeout = true;

    while (has_timeout) {
        uint32_t timer_id = 0;
        int type = 0;
    
        if (has_timeout) {
            DealwithTimeoutOne(timer_id, type);

            if (timeout != 0) {
                break;
            }
        }
    }
}

void EventLoop::DealwithTimeoutOne(const uint32_t timer_id, const int type) {
    auto it = mapping_timer_id_2_fd_.find(timer_id);
    if (it == mapping_timer_id_2_fd_.end()) {
        return;
    }

    int socket_fd = it->second;

    mapping_timer_id_2_fd_.erase(it);

    auto event_it = mapping_id_2_event_context_.find(socket_fd);
    if (event_it == mapping_id_2_event_context_.end()) {
        return;
    }

    event_it->second.event->OnTimeout(timer_id, type);
}

int EventLoop::GetActiveEventCount() {
    std::lock_guard<std::mutex> lck(mutex_);
    ClearEvent();
    return 0;
}

int EventLoop::Init(const int epoll_len) {
    epoll_fd_ = epoll_create(epoll_len);
    if (epoll_fd_ == -1) {
        return -1;
    }
    return 0;
}

void EventLoop::ModEvent(const Event* event, const int event_id) {
    auto it = mapping_id_2_event_context_.find(event->GetSocketFd());
    int epoll_operation = 0;
    if (it == mapping_id_2_event_context_.end()) {
        epoll_operation = EPOLL_CTL_ADD;
    }
    else {
        epoll_operation = it->second.event ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    }

    epoll_event tEpollEvent;
    tEpollEvent.events = event_id;
    tEpollEvent.data.fd = event->GetSocketFd();

    int ret = epoll_ctl(epoll_fd_, epoll_operation, event->GetSocketFd(), &tEpollEvent);
    if (ret == -1) {
        return;
    }

    EventCtx ctx;
    ctx.event = (Event *)event;
    ctx.event_ = event_id;
    
    mapping_id_2_event_context_[event->GetSocketFd()] = ctx;
}

void EventLoop::OnError(const int event_id, Event* event) {
    RemoveEvent(event);

    bool need_delete = false;
    event->OnError(need_delete);

    if (need_delete) {
        event->Destroy();
    }
}


void EventLoop::RemoveEvent(const Event * event) {
    auto it = mapping_id_2_event_context_.find(event->GetSocketFd());
    if (it == mapping_id_2_event_context_.end()) {
        return;
    }

    int epoll_operation = EPOLL_CTL_DEL;

    epoll_event tEpollEvent;
    tEpollEvent.events = 0;
    tEpollEvent.data.fd = event->GetSocketFd();

    int res = epoll_ctl(epoll_fd_, epoll_operation, event->GetSocketFd(), &tEpollEvent);
    if (res == -1) {
        return;
    }

    mapping_id_2_event_context_.erase(event->GetSocketFd());
}


void EventLoop::RemoveTimer(const uint32_t timer_id) {
    auto it = mapping_timer_id_2_fd_.find(timer_id);
    if (it != mapping_timer_id_2_fd_.end()) {
        mapping_timer_id_2_fd_.erase(it);
    }
}

void EventLoop::SingleLoop(const int timeout) {
    int n = epoll_wait(epoll_fd_, epoll_events, MAX_NUM_EVENTS, 1);
    if (n == -1) {
        if (errno != EINTR) {
            return;
        }
    }

    for (int i = 0; i < n; i++) {
        int iFd = epoll_events[i].data.fd;
        auto it = mapping_id_2_event_context_.find(iFd);
        if (it == mapping_id_2_event_context_.end()) {
            continue;
        }

        int event_id = epoll_events[i].events;
        Event* event = it->second.event;

        int res = 0;
        if (event_id & EPOLLERR) {
            OnError(event_id, event);
            continue;
        }
        
        try
        {
            if (event_id & EPOLLIN) {
                res = event->OnRead();
            }

            if (event_id & EPOLLOUT) {
                res = event->OnWrite();
            }
        }
        catch (...) {
            res = -1;
        }

        if (res != 0) {
            OnError(event_id, event);
        }
    }
}

void EventLoop::Start(){
    is_ended_ = false;
    while(true) {
        
        int timeout = 1000;
        
        DealwithTimeout(timeout);

        SingleLoop(timeout);

        CreateEvent();

        if (is_ended_) {
            break;
        }
    }
}

void EventLoop::Stop() {
    is_ended_ = true;
}

void EventLoop::SetTcpClient(TcpClient * poTcpClient) {
    tcp_client_ = poTcpClient;
}

void EventLoop::JumpoutEpollWait() {
    notify_->SendNotifyEvent();
}

}
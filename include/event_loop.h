#pragma once

#include "socket.h"
#include "notify_event.h"
#include <map>
#include <mutex>
#include <queue>
#include <sys/epoll.h>
#include <vector>

namespace paxos {

#define MAX_NUM_EVENTS 1024

class Event;
// class TcpAcceptor;
class TcpClient;
// class MessageEvent;
class Network;

class EventLoop {
public:
    EventLoop(Network * network);

    virtual ~EventLoop();

    int Init(const int iEpollLength);

    void ModEvent(const Event * poEvent, const int iEvents);

    void RemoveEvent(const Event * poEvent);

    void Start();

    void Stop();

    void OnError(const int iEvents, Event * poEvent);

    virtual void SingleLoop(const int iTimeoutMs);

    void SetTcpClient(TcpClient * tcp_client);

    void JumpoutEpollWait();

    bool AddTimer(const Event * poEvent, const int iTimeout, const int iType, uint32_t & iTimerID);

    void RemoveTimer(const uint32_t iTimerID);

    void DealwithTimeout(int& timeout);

    void DealwithTimeoutOne(const uint32_t iTimerID, const int iType);

    void AddEvent(int iFD, SocketAddress oAddr);

    void CreateEvent();

    void ClearEvent();

    int GetActiveEventCount();

public:
    typedef struct EventCtx
    {
        Event* event;
        int event_;
    } EventCtx_t;

protected:
    int epoll_fd_;
    epoll_event epoll_events[MAX_NUM_EVENTS];
    std::map<int, EventCtx_t> mapping_id_2_event_context_;
    Network * network;
    TcpClient * tcp_client_;
    NotifyEvent * notify_;
    //Timer m_oTimer;
    std::map<uint32_t, int> mapping_timer_id_2_fd_;

    std::queue<std::pair<int, SocketAddress> > queue_socket_addr_;
    std::mutex mutex_;
    //std::vector<MessageEvent *> m_vecCreatedEvent;

private:
    bool is_ended_;
};

}

#pragma once

#include "event.h"
#include "socket.h"
#include <mutex>

namespace paxos {

class EventLoop;
class Network;

enum MessageEventType {
    MessageEventType_RECV = 1,
    MessageEventType_SEND = 2,
};

enum MessageEventTimerType {
    MessageEventTimerType_Reconnect = 1,
};

class MessageEvent : public Event {
public: 
    MessageEvent(
        const int type, 
        const int fd, 
        const SocketAddress& addr,
        EventLoop* eventloop,
        Network* network
        );
    ~MessageEvent();

    int AddMessage(const std::string & message);

    int GetSocketFd() const;
    
    const std::string & GetSocketHost();

    int OnRead();

    int OnWrite();

    void OnTimeout(const uint32_t timer_id, const int type);

    void OnError(bool & need_delete);

    void OpenWrite();

    const bool IsActive();

private:
    int ReadLeft();

    //void ReadDone(BytesBuffer & oBytesBuffer, const int iLen);
    
    int WriteLeft();

    void WriteDone();

    int DoOnWrite();

    void ReConnect();

};

}
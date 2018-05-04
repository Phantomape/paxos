#pragma once

#include "concurrent.h"
#include "message_event.h"
#include "socket.h"
#include <string>
#include <vector>

namespace paxos {

class EventLoop;
class Network;

class TcpAcceptor : public Thread {
public:
    TcpAcceptor();
    ~TcpAcceptor();

    void Listen(const std::string& ip_listened, const int port_listened);

    void Run();

    void Stop();

    void AddEventLoop(EventLoop* poEventLoop);

    void AddEvent(int iFD, SocketAddress oAddr);

private:
    ServerSocket server_socket_;
    std::vector<EventLoop *> vec_event_loop_;

private:
    bool is_eneded_;
    bool is_started_;
};

}

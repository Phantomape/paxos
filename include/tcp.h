#pragma once

#include "concurrent.h"
#include "event_loop.h"
#include "tcp_acceptor.h"
#include "tcp_client.h"

namespace paxos {

class TcpRead : public Thread {
public:
    TcpRead(Network* network);
    
    ~TcpRead();

    int Init();

    void Run();

    void Stop();

    EventLoop * GetEventLoop();

private:
    EventLoop event_loop_;
};

class TcpWrite : public Thread
{
public:
    TcpWrite(Network* network);

    ~TcpWrite();

    int Init();

    void Run();

    void Stop();

    int AddMessage(const std::string& ip, const int port, const std::string& message);

private:
    TcpClient tcp_client_;
    EventLoop event_loop_;
};

class TcpIOThread 
{
public:
    TcpIOThread(Network* network);

    ~TcpIOThread();

    int Init(const std::string& ip_listened, const int port_listened, const int io_thread_count);

    void Start();

    void Stop();

    int AddMessage(const int group_idx, const std::string& ip, const int port, const std::string & message);

private:
    Network* network_;
    TcpAcceptor tcp_acceptor_;
    std::vector<TcpRead*> vec_tcp_read_;
    std::vector<TcpWrite*> vec_tcp_write_;
    bool is_started;
};

}
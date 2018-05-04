#include "network.h"
#include "tcp.h"
#include <assert.h>
#include <signal.h>

namespace paxos {

TcpRead::TcpRead(Network * network) : event_loop_(network) {
}

TcpRead::~TcpRead() {
}

int TcpRead::Init() {
    return event_loop_.Init(20480);
}

void TcpRead::Run() {
    event_loop_.Start();
}

void TcpRead::Stop() {
    event_loop_.Stop();
    Join();
}

EventLoop * TcpRead::GetEventLoop() {
    return &event_loop_;
}

TcpWrite::TcpWrite(Network * network)
    : tcp_client_(&event_loop_, network), event_loop_(network) {
    event_loop_.SetTcpClient(&tcp_client_);
}

TcpWrite::~TcpWrite() {
}

int TcpWrite::Init() {
    return event_loop_.Init(20480);
}

void TcpWrite::Run() {
    event_loop_.Start();
}

void TcpWrite::Stop() {
    event_loop_.Stop();
    Join();
}

int TcpWrite::AddMessage(const std::string & sIP, const int iPort, const std::string & sMessage)
{
    return tcp_client_.AddMessage(sIP, iPort, sMessage);
}

TcpIOThread::TcpIOThread(Network * network)
    : network_(network) {
    is_started = false;
    assert(signal(SIGPIPE, SIG_IGN) != SIG_ERR);
    assert(signal(SIGALRM, SIG_IGN) != SIG_ERR);
    assert(signal(SIGCHLD, SIG_IGN) != SIG_ERR);
}

TcpIOThread::~TcpIOThread() {
    for (auto & tcp_read : vec_tcp_read_) {
        delete tcp_read;
    }

    for (auto & tcp_write : vec_tcp_write_) {
        delete tcp_write;
    }
}

void TcpIOThread::Stop() {
    if (is_started) {
        tcp_acceptor_.Stop();
        for (auto & tcp_read : vec_tcp_read_) {
            tcp_read->Stop();
        }

        for (auto & tcp_write : vec_tcp_write_) {
            tcp_write->Stop();
        }
    }
}

int TcpIOThread::Init(const std::string & ip_listened, const int port_listened, const int io_thread_count) {
    for (int i = 0; i < io_thread_count; i++) {
        TcpRead* tcp_read = new TcpRead(network_);
        assert(tcp_read != nullptr);
        vec_tcp_read_.push_back(tcp_read);
        tcp_acceptor_.AddEventLoop(tcp_read->GetEventLoop());

        TcpWrite * tcp_write = new TcpWrite(network_);
        assert(tcp_write != nullptr);
        vec_tcp_write_.push_back(tcp_write);
    }

    tcp_acceptor_.Listen(ip_listened, port_listened);
    int ret = -1;

    for (auto & tcp_read : vec_tcp_read_) {
        ret = tcp_read->Init();
        if (ret != 0) {
            return ret;
        }
    }

    for (auto & tcp_write: vec_tcp_write_) {
        ret = tcp_write->Init();
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

void TcpIOThread::Start()
{
    tcp_acceptor_.Start();
    for (auto & tcp_write : vec_tcp_write_) {
        tcp_write->Start();
    }

    for (auto & tcp_read : vec_tcp_read_) {
        tcp_read->Start();
    }

    is_started = true;
}

int TcpIOThread::AddMessage(const int group_idx, const std::string& ip, const int port, const std::string& message) {
    int idx = group_idx % (int)vec_tcp_write_.size();
    return vec_tcp_write_[idx]->AddMessage(ip, port, message);
}

}

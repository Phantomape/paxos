#include "event_loop.h"
#include "tcp_acceptor.h"
#include <poll.h>
#include <stdio.h>

namespace paxos
{

TcpAcceptor::TcpAcceptor() {
    is_eneded_ = false;
    is_started_ = false;
}

TcpAcceptor::~TcpAcceptor() {}

void TcpAcceptor::Listen(const std::string& ip_listened, const int port_listened) {
    server_socket_.Listen(SocketAddress(ip_listened, (unsigned short)port_listened));
}

void TcpAcceptor::Stop() {
    if (is_started_) {
        is_eneded_ = true;
        Join();
    }
}

void TcpAcceptor::Run() {
    is_started_ = true;

    server_socket_.SetAcceptTimeout(500);
    server_socket_.SetNonBlocking(true);
    
    while (true) {
        struct pollfd pfd;
        int ret;

        pfd.fd =  server_socket_.GetSocketHandle();
        pfd.events = POLLIN;
        ret = poll(&pfd, 1, 500);

        if (ret != 0 && ret != -1) {
            SocketAddress oAddr;
            int fd = -1;
            try {
                fd = server_socket_.AcceptFd(&oAddr);
            }
            catch(...) {
                fd = -1;
            }

            if (fd >= 0) {
                AddEvent(fd, oAddr);
            }
        }

        if (is_eneded_) {
            return;
        }
    }
}

void TcpAcceptor::AddEventLoop(EventLoop* event_loop) {
    vec_event_loop_.push_back(event_loop);
}

void TcpAcceptor :: AddEvent(int iFD, SocketAddress oAddr) {
    EventLoop* min_active_event_loop = nullptr;
    int iMinActiveEventCount = 1 << 30;

    for (auto& event_loop : vec_event_loop_) {
        int active_event_count = event_loop->GetActiveEventCount();
        if (active_event_count < iMinActiveEventCount) {
            iMinActiveEventCount = active_event_count;
            min_active_event_loop = event_loop;
        }
    }

    min_active_event_loop->AddEvent(iFD, oAddr);
}

}

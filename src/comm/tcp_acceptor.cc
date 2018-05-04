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
    // m_oSocket.listen(SocketAddress(ip_listened, (unsigned short)port_listened));
}

void TcpAcceptor::Stop() {
    if (is_started_) {
        is_eneded_ = true;
        Join();
    }
}

void TcpAcceptor::Run()
{
    is_started_ = true;

    //m_oSocket.setAcceptTimeout(500);
    //m_oSocket.setNonBlocking(true);

    /*
    while (true) {
        struct pollfd pfd;
        int ret;

        pfd.fd =  m_oSocket.getSocketHandle();
        pfd.events = POLLIN;
        ret = poll(&pfd, 1, 500);

        if (ret != 0 && ret != -1)
        {
            SocketAddress oAddr;
            int fd = -1;
            try
            {
                fd = m_oSocket.acceptfd(&oAddr);
            }
            catch(...)
            {
                fd = -1;
            }
            
            if (fd >= 0)
            {
                BP->GetNetworkBP()->TcpAcceptFd();

                PLImp("accepted!, fd %d ip %s port %d",
                        fd, oAddr.getHost().c_str(), oAddr.getPort());

                AddEvent(fd, oAddr);
            }
        }

        if (m_bIsEnd)
        {
            PLHead("TCP.Acceptor [END]");
            return;
        }
    }
    */
}

void TcpAcceptor::AddEventLoop(EventLoop* event_loop) {
    vec_event_loop_.push_back(event_loop);
}

/*
void TcpAcceptor :: AddEvent(int iFD, SocketAddress oAddr)
{
    EventLoop * poMinActiveEventLoop = nullptr;
    int iMinActiveEventCount = 1 << 30;

    for (auto & poEventLoop : m_vecEventLoop)
    {
        int iActiveCount = poEventLoop->GetActiveEventCount();
        if (iActiveCount < iMinActiveEventCount)
        {
            iMinActiveEventCount = iActiveCount;
            poMinActiveEventLoop = poEventLoop;
        }
    }

    //printf("this %p addevent %p fd %d ip %s port %d\n", 
            //this, poMinActiveEventLoop, iFD, oAddr.getHost().c_str(), oAddr.getPort());
    poMinActiveEventLoop->AddEvent(iFD, oAddr);
}
*/
    
}



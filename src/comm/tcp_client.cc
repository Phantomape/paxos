#include "event_loop.h"
#include "network.h"
#include "tcp_client.h"
#include <arpa/inet.h>

namespace paxos {

TcpClient::TcpClient(EventLoop* eventloop, Network* network) 
    : eventloop_(eventloop), network_(network) {
        vec_event_.reserve(1000);
}

TcpClient::~TcpClient() {
    for (auto& it : map_event_) {
        delete it.second;
    }
}

int TcpClient::AddMessage(const std::string& ip, const int port, const std::string& message) {
    MessageEvent* event = GetEvent(ip, port);
    if (event == nullptr) {
        return -1;
    }

    return event->AddMessage(message);
}

MessageEvent* TcpClient::GetEvent(const std::string& ip, const int port)
{
    uint32_t ip_addr = (uint32_t)inet_addr(ip.c_str());
    uint64_t node_id = (((uint64_t)ip_addr) << 32) | port;

    std::lock_guard<std::mutex> lock_guard(mutex_);

    auto it = map_event_.find(node_id);
    if (it != map_event_.end()) {
        return it->second;
    }

    return CreateEvent(node_id, ip, port);
}

MessageEvent* TcpClient::CreateEvent(const uint64_t node_id, const std::string & ip, const int port)
{
    /*
    Socket oSocket;
    oSocket.setNonBlocking(true);
    oSocket.setNoDelay(true);
    SocketAddress oAddr(ip, port);
    oSocket.connect(oAddr);

    MessageEvent * event = new MessageEvent(MessageEventType_SEND, oSocket.detachSocketHandle(), 
            oAddr, m_eventLoop, m_poNetWork);
    assert(event != nullptr);

    map_event_[node_id] = event;
    vec_event_.push_back(event);

    return event;
    */
}

void TcpClient :: DealWithWrite()
{
    size_t size = vec_event_.size();

    for (size_t i = 0; i < size; i++) {   
        vec_event_[i]->OpenWrite();
    }
}

}

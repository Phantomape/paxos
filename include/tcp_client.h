#pragma once

#include "message_event.h"
#include <map>
#include <string>
#include <vector>

namespace paxos {

class EventLoop;
class Network;

class TcpClient {
public:
    TcpClient(EventLoop* event_loop, Network* network);
    ~TcpClient();

    int AddMessage(const std::string& ip, const int port, const std::string& message);

    void DealWithWrite();

private:
    MessageEvent* CreateEvent(const uint64_t node_id, const std::string& ip, const int port);
    MessageEvent* GetEvent(const std::string& ip, const int port);
    
    EventLoop* event_loop_;
    Network* network_;

    std::map<uint64_t, MessageEvent*> map_event_;
    std::vector<MessageEvent*> vec_event_;
    std::mutex mutex_;
};

}

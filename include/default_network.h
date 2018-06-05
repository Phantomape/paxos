#pragma once

#include "network.h"
#include "tcp.h"
#include "udp.h"
#include <string>

namespace paxos {

class DefaultNetwork : public Network {
public:
    DefaultNetwork();
    
    virtual ~DefaultNetwork();

    int Init(const std::string& ip, const int port, const int thread_cnt);

    void Run();

    void Stop();

    int SendMessageTCP(const int group_idx, const std::string& ip, const int port, const std::string& message);

    int SendMessageUDP(const int group_idx, const std::string& ip, const int port, const std::string& message);

private:
    UdpRecv udp_recv_;
    UdpSend udp_send_;
    TcpIOThread tcp_io_thread_;
};

}
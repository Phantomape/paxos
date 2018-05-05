#pragma once

#include "concurrent.h"

namespace paxos {

class DefaultNetwork;

struct Packet {
    std::string ip;
    int port;
    std::string message;
};

class UdpRecv : public Thread {
public:
    UdpRecv(DefaultNetwork* default_network);
    
    ~UdpRecv();

    int Init(const int port);

    void Run();

    void Stop();
    
private:
    DefaultNetwork* default_network;
    bool is_ended_;
    bool is_started_;
    int socket_file_descriptor_;
};

class UdpSend : public Thread {
public:
    UdpSend();
    
    ~UdpSend();

    int Init();

    void Run();

    void Stop();

    int AddMessage(const std::string& ip, const int port, const std::string& message);

private:
    void SendMessage(const std::string& ip, const int port, const std::string& message);

    Queue<Packet*> send_queue;
    bool is_ended_;
    bool is_started_;
    int socket_file_descriptor_;
};

}
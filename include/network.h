#pragma once

#include <string>

namespace paxos {

class Node;

class Network {
public:
    Network();
    virtual ~Network() {}

    //Network must not send/recieve any message before paxoslib called this funtion.
    virtual void Run() = 0;

    //If paxoslib call this function, network need to stop receive any message.
    virtual void Stop() = 0;

    virtual int SendMessageTCP(const int group_idx, const std::string& ip, const int port, const std::string& message) = 0;

    virtual int SendMessageUDP(const int group_idx, const std::string& ip, const int port, const std::string& message) = 0;

    //When receive a message, call this funtion.
    //This funtion is async, just enqueue an return.
    int OnReceiveMessage(const char* message, const int message_len);

private:
    friend class Node;
    Node* node;
};
    
}

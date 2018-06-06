#pragma once

#include "config.h"
#include "network.h"
#include "options.h"
#include "communicate.h"
#include <map>

namespace paxos {

class Communicator : public Communicate {
public:
    Communicator(
            const Config * config,
            const uint64_t node_id,
            const int udp_max_size,
            Network * network);
    ~Communicator();

    int SendMessage(
            const int group_idx,
            const uint64_t iSendtoNodeID,
            const std::string & message,
            const int iSendType = Message_SendType_UDP
        );

    int BroadcastMessage(
            const int group_idx,
            const std::string & message,
            const int iSendType = Message_SendType_UDP
        );

    int BroadcastMessageFollower(
            const int group_idx,
            const std::string & message,
            const int iSendType = Message_SendType_UDP
        );

    int BroadcastMessageTempNode(
            const int group_idx,
            const std::string & message,
            const int iSendType = Message_SendType_UDP
        );

public:
    void SetUDPMaxSize(const size_t udp_max_size);

private:
    int Send(
            const int group_idx,
            const uint64_t node_id,
            const NodeInfo & node_info,
            const std::string & message,
            const int iSendType
        );

private:
    Config * config_;
    Network * network_;

    uint64_t node_id_;
    size_t udp_max_size_; 
};

}

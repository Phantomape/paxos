#pragma once

#include "options.h"

namespace paxos {

enum Message_SendType {
    Message_SendType_UDP = 0,
    Message_SendType_TCP = 1,
};

class Communicate {
public:
    virtual ~Communicate() {}

    virtual int SendMessage(
        const int group_idx, 
        const uint64_t recv_node_id, 
        const std::string & buffer, 
        const int send_type = Message_SendType_UDP
    ) = 0;

    virtual int BroadcastMessage(
        const int group_idx, 
        const std::string & buffer, 
        const int send_type = Message_SendType_UDP
    ) = 0;
    
    virtual int BroadcastMessageFollower(
        const int group_idx, 
        const std::string & buffer, 
        const int send_type = Message_SendType_UDP
    ) = 0;
    
    virtual int BroadcastMessageTempNode(
        const int group_idx, 
        const std::string & buffer, 
        const int send_type = Message_SendType_UDP
    ) = 0;
};

}
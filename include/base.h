#pragma once

#include "paxos_msg.pb.h"

namespace paxos {

class Base {
public: 
    Base();
    virtual ~Base();

    virtual int BroadcastMessage(const PaxosMsg &paxos_msg);
};

}
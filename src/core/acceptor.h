#pragma once

#include "base.h"

namespace paxos {

class Acceptor {
public: 
    Acceptor();
    ~Acceptor();

    void OnPrepare(const PaxosMsg &recv_paxos_msg);
};

}
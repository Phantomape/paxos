#pragma once

#include "base.h"

namespace paxos {

class Acceptor : public Base {
public: 
    Acceptor();
    ~Acceptor();

    void Init();
    virtual void InitInstance();

    void OnPrepare(const PaxosMsg &recv_paxos_msg);
    void OnAccept(const PaxosMsg &paxos_msg);
};

}
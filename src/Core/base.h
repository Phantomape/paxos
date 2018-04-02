#pragma once

namespace paxos {

class Base {
public: 
    Base();
    virtual ~Base();

    virtual int BroadcastMessage();
};

}
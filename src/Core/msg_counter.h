#pragma once

#include <string>
#include <set>

namespace paxos {

class MsgCounter {
public:
    MsgCounter();
    ~MsgCounter();

    void AddAcceptedMsg();
    void AddRejectedMsg();
}

}
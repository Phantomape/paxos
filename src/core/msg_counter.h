#pragma once

#include <string>
#include <set>

namespace paxos {

class MsgCounter {
public:
    MsgCounter();
    ~MsgCounter();

    void AddAcceptedMsg();
    void AddReceivedMsg();
    void AddRejectedMsg();

    bool IsPassed();
    bool IsRejected();
};

}
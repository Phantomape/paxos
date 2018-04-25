#pragma once

#include <string>
#include <set>

namespace paxos {

class MsgCounter {
public:
    MsgCounter();
    ~MsgCounter();

    void AddAcceptedMsg(const uint64_t node_id);
    void AddReceivedMsg(const uint64_t node_id);
    void AddRejectedMsg(const uint64_t node_id);

    bool IsPassed();
    bool IsRejected();
};

}
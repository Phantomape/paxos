#pragma once

#include <string>
#include <set>

namespace paxos {

class Config;
class PaxosLog;

class MsgCounter {
public:
    MsgCounter();
    MsgCounter(const Config * poConfig);
    ~MsgCounter();

    void AddAcceptedMsg(const uint64_t node_id);
    void AddReceivedMsg(const uint64_t node_id);
    void AddRejectedMsg(const uint64_t node_id);

    void Init();

    bool IsPassed();
    bool IsRejected();
    bool IsAllReceived();

    std::set<uint64_t> recv_msg_node_ids;
    std::set<uint64_t> rej_msg_node_ids;
    std::set<uint64_t> ac_msg_node_ids;

    Config* config_;
};

}
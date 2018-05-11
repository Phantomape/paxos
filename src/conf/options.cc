#include "options.h"
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

namespace paxos {

NodeInfo::NodeInfo()
    : node_id_(nullnode), ip_(""), port_(-1) {
}

NodeInfo::NodeInfo(const uint64_t iNodeId)
    : node_id_(iNodeId), ip_(""), port_(-1) {
    ParseNodeId();
}

NodeInfo::NodeInfo(const std::string & ip, const int port)
    : node_id_(nullnode), ip_(ip), port_(port) {
    CreateNodeId();
}

const uint64_t NodeInfo::GetNodeId() const {
    return node_id_;
}

const std::string & NodeInfo::GetIp() const {
    return ip_;
}

const int NodeInfo::GetPort() const {
    return port_;
}

void NodeInfo::SetIpPort(const std::string & ip, const int port) {
    ip_ = ip;
    port_ = port;
    CreateNodeId();
}

void NodeInfo::SetNodeId(const uint64_t iNodeId) {
    node_id_ = iNodeId;
    ParseNodeId();
}

void NodeInfo::CreateNodeId() {
    uint32_t ip = (uint32_t)inet_addr(ip_.c_str());
    assert(ip != (uint32_t)-1);

    node_id_ = (((uint64_t)ip) << 32) | port_;

    //PLImp("ip %s ip %u port %d nodeid %lu", ip_.c_str(), ip, port_, node_id_);
}

void NodeInfo::ParseNodeId() {
    port_ = node_id_ & (0xffffffff);

    in_addr addr;
    addr.s_addr = node_id_ >> 32;
    
    ip_ = std::string(inet_ntoa(addr));

    //PLImp("nodeid %lu ip %s ip %u port %d", node_id_, ip_.c_str(), addr.s_addr, port_);
}

GroupStateMachineInfo::GroupStateMachineInfo() {
    group_idx_ = -1;
    use_master_ = false;
}

Options::Options() {
    log_storage_ = nullptr;
    sync_ = true;
    sync_interval_ = 0;
    network_ = nullptr;
    udp_max_msg_size_ = 4096;
    io_thread_count_ = 1;
    group_count_ = 1;
    use_membership_ = false;
    membership_change_callback_ = nullptr;
    master_change_callback_ = nullptr;
    //poBreakpoint = nullptr;
    is_large_value_mode_ = false;
    log_func_ = nullptr;
    log_level_ = LogLevel::LogLevel_None;
    use_checkpoint_replayer_ = false;
    use_batch_propose_ = false;
    open_change_value_before_propose_ = false;
}
    
}



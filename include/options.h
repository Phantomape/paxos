#pragma once

#include <functional>
#include <string>
#include <vector>

namespace paxos {

class LogStorage;
class Network;
class StateMachine;

static const uint64_t nullnode = 0;

class NodeInfo
{
public:
    NodeInfo(); 

    NodeInfo(const uint64_t node_id);

    NodeInfo(const std::string & sIP, const int iPort); 
    
    virtual ~NodeInfo() { }

    const uint64_t GetNodeId() const;

    const std::string& GetIp() const;
    
    const int GetPort() const;

    void SetIpPort(const std::string& ip, const int port);

    void SetNodeId(const uint64_t node_id);

private:
    void CreateNodeId();
    void ParseNodeId();

    uint64_t node_id_;
    std::string ip_;
    int port_;
};

class FollowerNodeInfo {
public:
    NodeInfo node_;
    NodeInfo follow_node_;
};

typedef std::vector<NodeInfo> NodeInfoList;
typedef std::vector<FollowerNodeInfo> FollowerNodeInfoList;

class GroupStateMachineInfo {
public:
    GroupStateMachineInfo();

    //required
    //GroupIdx interval is [0, group_count_) 
    int group_idx_;

    //optional
    //One paxos group can mounting multi state machines.
    std::vector<StateMachine*> vec_state_machine_list_;

    //optional
    //Master election is a internal state machine. 
    //Set use_master_ as true to open master election feature.
    //Default is false.
    bool use_master_;
};

typedef std::vector<GroupStateMachineInfo> GroupStateMachineInfoList;

typedef std::function< void(const int, NodeInfoList &) > MembershipChangeCallback;
typedef std::function< void(const int, const NodeInfo &, const uint64_t) > MasterChangeCallback;

class Options {
public:
    Options();

    //optional
    //User-specified paxoslog storage.
    //Default is nullptr.
    LogStorage* log_storage_;

    //optional
    //If log_storage_ == nullptr, log_storage_path_ is required. 
    std::string log_storage_path_;

    //optional
    //If true, the write will be flushed from the operating system
    //buffer cache before the write is considered complete.
    //If this flag is true, writes will be slower.
    //
    //If this flag is false, and the machine crashes, some recent
    //writes may be lost. Note that if it is just the process that
    //crashes (i.e., the machine does not reboot), no writes will be
    //lost even if sync==false. Because of the data lost, we do not 
    //guarantee consistence.
    //
    //Default is true.
    bool sync_;

    //optional
    //Default is 0.
    //This means the write will skip flush at most sync_interval_ times.
    //That also means you will lost at most sync_interval_ count's paxos log.
    int sync_interval_;

    //optional
    //User-specified network.
    Network* network_;

    //optional
    //Our default network use udp and tcp combination, a message we use udp or tcp to send decide by a threshold.
    //Message size under udp_max_msg_size_ we use udp to send.
    //Default is 4096.
    size_t udp_max_msg_size_;

    //optional
    //Our default network io thread count.
    //Default is 1.
    int io_thread_count_;
    
    //optional
    //We support to run multi phxpaxos on one process.
    //One paxos group here means one independent phxpaxos. Any two phxpaxos(paxos group) only share network, no other.
    //There is no communication between any two paxos group.
    //Default is 1.
    int group_count_;

    //required
    //Self node's ip/port.
    NodeInfo node_;

    //required
    //All nodes's ip/port with a paxos set(usually three or five nodes).
    NodeInfoList vec_node_info_list_;
    
    //optional
    //Only use_member_ship_ == true, we use option's nodeinfolist to init paxos 
    //membership, after that, paxos will remember all nodeinfos, so second time 
    //you can run paxos without vecNodeList, and you can only change membership 
    //by using function in node.h. 
    //
    //Default is false.
    //if use_member_ship_ == false, that means every time you run paxos will use 
    //vecNodeList to build a new membership.when you change membership by a new 
    //vecNodeList, we don't guarantee consistence.
    //
    //For test, you can set false.
    //But when you use it to real services, remember to set true. 
    bool use_membership_;

    //While membership change, phxpaxos will call this function.
    //Default is nullptr.
    MembershipChangeCallback membership_change_callback_;

    //While master change, phxpaxos will call this function.
    //Default is nullptr.
    MasterChangeCallback master_change_callback_;

    //optional
    //One phxpaxos can mounting multi state machines.
    //This vector include different phxpaxos's state machines list.
    GroupStateMachineInfoList vec_group_state_machine_info_list_;

    //optional
    //Breakpoint * poBreakpoint;

    //optional
    //If use this mode, that means you propose large value(maybe large than 5M 
    //means large) much more. Large value means long latency, long timeout, this 
    //mode will fit it.
    //Default is false
    bool is_large_value_mode_;

    //optional
    //All followers's ip/port, and follow to node's ip/port.
    //Follower only learn but do not participate paxos algorithmic process.
    //Default is empty.
    FollowerNodeInfoList vec_follower_node_info_list_;

    //optional
    //Notice, this function must be thread safe!
    //if pLogFunc == nullptr, we will print log to standard ouput.
    //LogFunc pLogFunc;

    //optional
    //If you use your own log function, then you control loglevel yourself, ignore this.
    //Check log.h to find 5 level.
    //Default is LogLevel::LogLevel_None, that means print no log.
    //LogLevel eLogLevel;

    //optional
    //If you use checkpoint replayer feature, set as true.
    //Default is false;
    bool use_checkpoint_replayer_;

    //optional
    //Only use_batch_propose_ is true can use API BatchPropose in node.h
    //Default is false;
    bool use_batch_propose_;

    //optional
    //Only open_change_value_before_propose_ is true, that will callback sm's function(BeforePropose).
    //Default is false;
    bool open_change_value_before_propose_;
};
    
}

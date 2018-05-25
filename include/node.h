#pragma once

#include "internal_options.h"
#include "options.h"
#include "state_machine.h"
#include <string>
#include <vector>

namespace paxos {

class Network;

class Node {
public:
    Node() {}
    
    virtual ~Node() {}

    static int Run(const Options& options, Node*& node);

    //Base function.
    virtual int Propose(const int group_idx, const std::string& val, uint64_t& instance_id) = 0;

    virtual int Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, StateMachineCtx * poSMCtx) = 0;

    virtual const uint64_t GetCurrentInstanceId(const int group_idx) = 0;

    virtual const uint64_t GetMinChosenInstanceId(const int group_idx) = 0;

    virtual const uint64_t GetNodeId() const = 0;

    //Batch propose.
    
    //This feature is enabled only after setting options::bUserBatchPropose as true can use this batch API.
    //Warning: BatchProposal will have same instance_id returned but different batch_idx.
    //Batch values's execute order in StateMachine is certain, the return value batch_idx
    //means the execute order index, start from 0.
    virtual int BatchPropose(const int group_idx, const std::string & val, 
            uint64_t & instance_id, uint32_t & batch_idx) = 0;

    //virtual int BatchPropose(const int group_idx, const std::string & val, uint64_t & instance_id, 
    //    uint32_t & batch_idx, SMCtx * poSMCtx) = 0;

    //PhxPaxos will batch proposal while waiting proposals count reach to BatchCount, 
    //or wait time reach to BatchDelayTimeMs.
    virtual void SetBatchCount(const int group_idx, const int batch_count) = 0;

    virtual void SetBatchDelayTimeMs(const int group_idx, const int batch_delay_time_ms) = 0;

    //State machine.
    
    //This function will add state machine to all group.
    //virtual void AddStateMachine(StateMachine * poSM) = 0;
    
    //virtual void AddStateMachine(const int group_idx, StateMachine * poSM) = 0;

    //Timeout control.
    virtual void SetTimeoutMs(const int timeout_ms) = 0;

    //Checkpoint
    
    //Set the number you want to keep paxoslog's count.
    //We will only delete paxoslog before checkpoint instanceid.
    //If hold_count < 300, we will set it to 300. Not suggest too small holdcount.
    virtual void SetHoldPaxosLogCount(const uint64_t hold_count) = 0;

    //Replayer is to help sm make checkpoint.
    //Checkpoint replayer default is paused, if you not use this, ignord this function.
    //If sm use ExecuteForCheckpoint to make checkpoint, you need to run replayer(you can run in any time).
    
    //Pause checkpoint replayer.
    virtual void PauseCheckpointReplayer() = 0;

    //Continue to run replayer
    virtual void ContinueCheckpointReplayer() = 0;

    //Paxos log cleaner working for deleting paxoslog before checkpoint instanceid.
    //Paxos log cleaner default is pausing.
    
    //pause paxos log cleaner.
    virtual void PausePaxosLogCleaner() = 0;

    //Continue to run paxos log cleaner.
    virtual void ContinuePaxosLogCleaner() = 0;

    //Membership
    
    //Show now membership.
    //virtual int ShowMembership(const int group_idx, NodeInfoList & vecNodeInfoList) = 0;
    
    //Add a paxos node to membership.
    //virtual int AddMember(const int group_idx, const NodeInfo & oNode) = 0;

    //Remove a paxos node from membership.
    //virtual int RemoveMember(const int group_idx, const NodeInfo & oNode) = 0;

    //Change membership by one node to another node.
    //virtual int ChangeMember(const int group_idx, const NodeInfo & oFromNode, const NodeInfo & oToNode) = 0;

    //Master
    
    //Check who is master.
    //virtual const NodeInfo GetMaster(const int group_idx) = 0;

    //Check who is master and get version.
    //virtual const NodeInfo GetMasterWithVersion(const int group_idx, uint64_t & llVersion) = 0;
    
    //Check is i'm master.
    virtual const bool IsIMMaster(const int group_idx) = 0;

    virtual int SetMasterLease(const int group_idx, const int lease_time_ms) = 0;

    virtual int DropMaster(const int group_idx) = 0;

    //Qos

    //If many threads propose same group, that some threads will be on waiting status.
    //Set max hold threads, and we will reject some propose request to avoid to many threads be holded.
    //Reject propose request will get retcode(PaxosTryCommitRet_TooManyThreadWaiting_Reject), check on def.h.
    virtual void SetMaxHoldThreads(const int group_idx, const int max_num_hold_threads) = 0;

    //To avoid threads be holded too long time, we use this threshold to reject some propose to control thread's wait time.
    virtual void SetProposeWaitTimeThresholdMS(const int group_idx, const int wait_time_threshold_ms) = 0;

    //write disk
    virtual void SetLogSync(const int group_idx, const bool log_sync) = 0;

    //Not suggest to use this function
    //pair: value,smid.
    //Because of BatchPropose, a InstanceId maybe include multi-value.
    virtual int GetInstanceValue(const int group_idx, const uint64_t instance_id, 
            std::vector<std::pair<std::string, int> > & vec_vals) = 0;

protected:
    friend class Network; 

    virtual int OnReceiveMessage(const char * message, const int message_len) = 0;
};

}

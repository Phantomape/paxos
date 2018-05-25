#pragma once

#include <string>
#include <vector>
#include <typeinfo>
#include <stdint.h>
#include <inttypes.h>

namespace paxos {

class StateMachineCtx {
public:
    StateMachineCtx();

    StateMachineCtx(const int state_machine_id, void* ctx);

    int state_machine_id_;
    void* ctx_;
};

class CheckpointFileInfo {
public:
    std::string file_path_;
    size_t file_size_;
};

typedef std::vector<CheckpointFileInfo> CheckpointFileInfoList;

const uint64_t NoCheckpoint = static_cast<uint64_t>(-1); 

class StateMachine {
public:
    virtual ~StateMachine() {}

    //Different state machine return different StateMachineId().
    virtual const int StateMachineId() const = 0;

    //Return true means execute success. 
    //This 'success' means this execute don't need to retry.
    //Sometimes you will have some logical failure in your execute logic, 
    //and this failure will definite occur on all node, that means this failure 
    //is acceptable, for this case, return true is the best choice.
    //Some system failure will let different node's execute result inconsistent,
    //for this case, you must return false to retry this execute to avoid this 
    //system failure. 
    virtual bool Execute(
        const int group_idx, 
        const uint64_t instance_id, 
        const std::string& val, 
        StateMachineCtx* state_machine_ctx
        ) = 0;

    virtual bool ExecuteForCheckpoint(
        const int group_idx, 
        const uint64_t instance_id, 
        const std::string & val
        );

    //Only need to implement this function while you have checkpoint.
    //Return your checkpoint's max executed instanceid.
    //Notice PhxPaxos will call this function very frequently.
    virtual const uint64_t GetCheckpointInstanceId(const int group_idx) const;

    //After called this function, the vec_file_list that GetCheckpointState return's, can't be delelted, moved and modifyed.
    virtual int LockCheckpointState();
    
    //sDirpath is checkpoint data root dir path.
    //vec_file_list is the relative path of the dir_path.
    virtual int GetCheckpointState(
        const int group_idx, 
        std::string & dir_path, 
        std::vector<std::string> & vec_file_list
        ); 

    virtual void UnlockCheckpointState();
    
    //Checkpoint file was on dir(checkpoint_tmp_file_dir_path).
    //vec_file_list is all the file in dir(checkpoint_tmp_file_dir_path).
    //vec_file_list filepath is absolute path.
    //After called this fuction, paxoslib will kill the processor. 
    //State machine need to understand this when restart.
    virtual int LoadCheckpointState(
        const int group_idx, 
        const std::string & checkpoint_tmp_file_dir_path,
        const std::vector<std::string> & vec_file_list, 
        const uint64_t checkpoint_instance_id
        );

    //You can modify your request at this moment.
    //At this moment, the state machine data will be up to date.
    //If request is batch, propose requests for multiple identical state machines 
    //will only call this function once. Ensure that the execute function 
    //correctly recognizes the modified request. Since this function is not always 
    //called, the execute function must handle the unmodified request correctly.
    virtual void BeforePropose(const int group_idx, std::string& val);

    //Because function BeforePropose much waste cpu,
    //Only NeedCallBeforePropose return true then weill call function BeforePropose.
    //You can use this function to control call frequency.
    //Default is false.
    virtual const bool NeedCallBeforePropose();
};
    
}

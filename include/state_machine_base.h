#pragma once

#include "state_machine.h"
#include <vector>

namespace paxos {


class BatchStateMachineCtx {
public:
    std::vector<StateMachineCtx *> vec_state_machine_ctx_list_;
};

class StateMachineFac {
public:
    StateMachineFac(const int group_idx);

    ~StateMachineFac();

    bool Execute(
        const int group_idx,
        const uint64_t instance_id,
        const std::string& val,
        StateMachineCtx* state_machine_ctx
        );

    bool ExecuteForCheckpoint(
        const int group_idx,
        const uint64_t instance_id,
        const std::string & val
        );

    void PackPaxosValue(std::string & val,const int state_machine_id = 0);

    void AddStateMachine(StateMachine * state_machine);

    void BeforePropose(const int group_idx,std::string & val);

    void BeforeBatchPropose(const int group_idx,std::string & val);

    void BeforeProposeCall(
        const int group_idx,
        const int state_machine_id,
        std::string & val,
        bool & change
        );

    const uint64_t GetCheckpointInstanceId(const int group_idx) const;

    std::vector<StateMachine *> GetStateMachineList();

private:
    bool BatchExecute(
        const int group_idx,
        const uint64_t instance_id,
        const std::string & val,
        BatchStateMachineCtx * batch_state_machine_ctx
        );

    bool DoExecute(
        const int group_idx,
        const uint64_t instance_id,
        const std::string & val,
        const int state_machine_id,
        StateMachineCtx * state_machine_ctx
        );

    bool BatchExecuteForCheckpoint(
        const int group_idx,
        const uint64_t instance_id,
        const std::string & val
        );

    bool DoExecuteForCheckpoint(
        const int group_idx,
        const uint64_t instance_id,
        const std::string & val,
        const int state_machine_id
        );

private:
    std::vector<StateMachine *> vec_state_machine_list_;
    int group_idx_;
};
}
#pragma once

#include "../include/state_machine.h"
#include "../include/options.h"
#include <cstdint>
#include <stdio.h>
#include <unistd.h>

namespace benchmark {

class BenchmarkStateMachine : public paxos::StateMachine {
public:
    BenchmarkStateMachine(const uint64_t llMyNodeID, const int iGroupIdx);

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, paxos::StateMachineCtx * poSMCtx);

    const int StateMachineId() const { return 1; }

    bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue) { return true; }

    const uint64_t GetCheckpointInstanceId(const int iGroupIdx) const { return paxos::NoCheckpoint; }

    const int GetGroupIdx() const;

    int GetCheckpointState(const int iGroupIdx, std::string & sDirPath, 
            std::vector<std::string> & vecFileList) { return 0; }

    int LoadCheckpointState(const int iGroupIdx, const std::string & sCheckpointTmpFileDirPath,
            const std::vector<std::string> & vecFileList, const uint64_t llCheckpointInstanceID) { return 0; }

    int LockCheckpointState() { return 0; }

    void UnLockCheckpointState() { }

private:
    uint64_t node_id_;
    int group_idx_;
};

}
#include "benchmark_state_machine.h"

namespace benchmark {

BenchmarkStateMachine::BenchmarkStateMachine(const uint64_t llMyNodeID, const int iGroupIdx)
    : node_id_(llMyNodeID), group_idx_(iGroupIdx) {
}

bool BenchmarkStateMachine::Execute(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sPaxosValue, paxos::StateMachineCtx * poSMCtx) {
    //bench sm do nothing
    return true;
}

const int BenchmarkStateMachine::GetGroupIdx() const {
    return group_idx_;
}

}

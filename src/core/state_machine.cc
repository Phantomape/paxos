#include "state_machine.h"

namespace paxos {

StateMachineCtx::StateMachineCtx(const int state_machine_id, void * ctx) 
    : state_machine_id_(state_machine_id), ctx_(ctx) {
}

StateMachineCtx::StateMachineCtx() : state_machine_id_(0), ctx_(nullptr) {
}

bool StateMachine::ExecuteForCheckpoint(const int group_idx, const uint64_t instance_id, 
        const std::string & val) { 
    return true;
}

const uint64_t StateMachine::GetCheckpointInstanceId(const int group_idx) const { 
    return paxos::NoCheckpoint;
}

//default no checkpoint 
int StateMachine::GetCheckpointState(const int group_idx, std::string & dir_path, 
        std::vector<std::string> & vec_file_list) {
    return -1;
}

int StateMachine::LoadCheckpointState(const int group_idx, const std::string & checkpoint_tmp_file_dir_path,
        const std::vector<std::string> & vec_file_list, const uint64_t checkpoint_instance_id) {
    return -1;
}

int StateMachine::LockCheckpointState() {
    return -1;
}

void StateMachine::UnlockCheckpointState() { 
}

void StateMachine::BeforePropose(const int group_idx, std::string & val) {
}

const bool StateMachine::NeedCallBeforePropose() {
    return false;
}

}

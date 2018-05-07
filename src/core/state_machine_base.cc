#include "def.h"
#include "paxos_msg.pb.h"
#include "state_machine_base.h"
#include <set>
#include <string.h>

namespace paxos {

StateMachineFac::StateMachineFac(const int group_idx) : group_idx_(group_idx)
{
}

StateMachineFac::~StateMachineFac()
{
}

bool StateMachineFac::Execute(const int group_idx, const uint64_t instance_id, const std::string & val, StateMachineCtx * state_machine_ctx) {
    if (val.size() < sizeof(int)) {
        return true;
    }

    int state_machine_id = 0;
    memcpy(&state_machine_id, val.data(), sizeof(int));

    if (state_machine_id == 0) {
        return true;
    }

    std::string sBodyValue = std::string(val.data() + sizeof(int), val.size() - sizeof(int));
    if (state_machine_id == BATCH_PROPOSE_STATE_MACHINE_ID) {
        BatchStateMachineCtx * batch_state_machine_ctx = nullptr;
        if (state_machine_ctx != nullptr && state_machine_ctx->ctx_ != nullptr) {
            batch_state_machine_ctx = (BatchStateMachineCtx *)state_machine_ctx->ctx_;
        }
        return BatchExecute(group_idx, instance_id, sBodyValue, batch_state_machine_ctx);
    }
    else
    {
        return DoExecute(group_idx, instance_id, sBodyValue, state_machine_id, state_machine_ctx);
    }
}

bool StateMachineFac::BatchExecute(const int group_idx, const uint64_t instance_id, const std::string & sBodyValue, BatchStateMachineCtx * batch_state_machine_ctx) {
    BatchPaxosValues oBatchValues;
    bool is_succeeded = oBatchValues.ParseFromArray(sBodyValue.data(), sBodyValue.size());
    if (!is_succeeded) {
        return false;
    }

    if (batch_state_machine_ctx != nullptr) {
        if ((int)batch_state_machine_ctx->vec_state_machine_ctx_list_.size() != oBatchValues.values_size()) {
            return false;
        }
    }

    for (int i = 0; i < oBatchValues.values_size(); i ++) {
        const PaxosValue & oValue = oBatchValues.values(i);
        StateMachineCtx * state_machine_ctx = batch_state_machine_ctx != nullptr ? batch_state_machine_ctx->vec_state_machine_ctx_list_[i] : nullptr;
        bool bExecuteSucc = DoExecute(group_idx, instance_id, oValue.value(), oValue.smid(), state_machine_ctx);
        if (!bExecuteSucc) {
            return false;
        }
    }

    return true;
}

bool StateMachineFac::DoExecute(const int group_idx, const uint64_t instance_id, 
        const std::string & sBodyValue, const int state_machine_id, StateMachineCtx * state_machine_ctx) {
    if (state_machine_id == 0) {
        return true;
    }

    if (vec_state_machine_list_.size() == 0) {
        return false;
    }

    for (auto & state_machine : vec_state_machine_list_) {
        if (state_machine->StateMachineId() == state_machine_id) {
            return state_machine->Execute(group_idx, instance_id, sBodyValue, state_machine_ctx);
        }
    }

    return false;
}


bool StateMachineFac::ExecuteForCheckpoint(const int group_idx, const uint64_t instance_id, const std::string & val) {
    if (val.size() < sizeof(int)) {
        return true;
    }

    int state_machine_id = 0;
    memcpy(&state_machine_id, val.data(), sizeof(int));

    if (state_machine_id == 0) {
        return true;
    }

    std::string sBodyValue = std::string(val.data() + sizeof(int), val.size() - sizeof(int));
    if (state_machine_id == BATCH_PROPOSE_STATE_MACHINE_ID) {
        return BatchExecuteForCheckpoint(group_idx, instance_id, sBodyValue);
    }
    else {
        return DoExecuteForCheckpoint(group_idx, instance_id, sBodyValue, state_machine_id);
    }
}

bool StateMachineFac::BatchExecuteForCheckpoint(const int group_idx, const uint64_t instance_id, 
        const std::string & sBodyValue) {
    BatchPaxosValues oBatchValues;
    bool is_succeeded = oBatchValues.ParseFromArray(sBodyValue.data(), sBodyValue.size());
    if (!is_succeeded) {   
        return false;
    }

    for (int i = 0; i < oBatchValues.values_size(); i ++) {
        const PaxosValue & oValue = oBatchValues.values(i);
        bool bExecuteSucc = DoExecuteForCheckpoint(group_idx, instance_id, oValue.value(), oValue.smid());
        if (!bExecuteSucc) {
            return false;
        }
    }

    return true;
}

bool StateMachineFac::DoExecuteForCheckpoint(const int group_idx, const uint64_t instance_id, 
        const std::string & sBodyValue, const int state_machine_id) {
    if (state_machine_id == 0) {
        return true;
    }

    if (vec_state_machine_list_.size() == 0) {
        return false;
    }

    for (auto & state_machine : vec_state_machine_list_) {
        if (state_machine->StateMachineId() == state_machine_id) {
            return state_machine->ExecuteForCheckpoint(group_idx, instance_id, sBodyValue);
        }
    }

    return false;
}


void StateMachineFac::PackPaxosValue(std::string & val, const int state_machine_id) {
    char sStateMachineID[sizeof(int)] = {0};
    if (state_machine_id != 0)
    {
        memcpy(sStateMachineID, &state_machine_id, sizeof(sStateMachineID));
    }

    val = std::string(sStateMachineID, sizeof(sStateMachineID)) + val;
}

void StateMachineFac::AddStateMachine(StateMachine * state_machine) {
    for (auto & state_machinet : vec_state_machine_list_) {
        if (state_machinet->StateMachineId() == state_machine->StateMachineId()) {
            return;
        }
    }

    vec_state_machine_list_.push_back(state_machine);
}

const uint64_t StateMachineFac::GetCheckpointInstanceId(const int group_idx) const {
    uint64_t checkpoint_instance_id = -1;
    uint64_t checkpoint_instance_id_Insize = -1;
    bool use_state_machine = false;

    for (auto & state_machine : vec_state_machine_list_) {
        uint64_t checkpoint_instance_id = state_machine->GetCheckpointInstanceId(group_idx);
        if (state_machine->StateMachineId() == SYSTEM_V_STATE_MACHINE_ID
                || state_machine->StateMachineId() == MASTER_V_STATE_MACHINE_ID) {
            //system variables 
            //master variables
            //if no user state machine, system and master's can use.
            //if have user state machine, use user'state machine's checkpointinstanceid.
            if (checkpoint_instance_id == uint64_t(-1)) {
                continue;
            }
            
            if (checkpoint_instance_id > checkpoint_instance_id_Insize
                    || checkpoint_instance_id_Insize == (uint64_t)-1) {
                checkpoint_instance_id_Insize = checkpoint_instance_id;
            }

            continue;
        }

        use_state_machine = true;

        if (checkpoint_instance_id == uint64_t(-1)) {
            continue;
        }
        
        if (checkpoint_instance_id > checkpoint_instance_id
                || checkpoint_instance_id == (uint64_t)-1) {
            checkpoint_instance_id = checkpoint_instance_id;
        }
    }
    
    return use_state_machine ? checkpoint_instance_id : checkpoint_instance_id_Insize;
}

std::vector<StateMachine *> StateMachineFac::GetStateMachineList() {
    return vec_state_machine_list_;
}

void StateMachineFac::BeforePropose(const int group_idx, std::string & sValue)
{
    int state_machine_id = 0;
    memcpy(&state_machine_id, sValue.data(), sizeof(int));

    if (state_machine_id == 0) {
        return;
    }

    if (state_machine_id == BATCH_PROPOSE_STATE_MACHINE_ID) {
        BeforeBatchPropose(group_idx, sValue);
    }
    else {
        bool change = false;
        std::string sBodyValue = std::string(sValue.data() + sizeof(int), sValue.size() - sizeof(int));
        BeforeProposeCall(group_idx, state_machine_id, sBodyValue, change);
        if (change) {
            sValue.erase(sizeof(int));
            sValue.append(sBodyValue);
        }
    }
}

void StateMachineFac::BeforeBatchPropose(const int group_idx, std::string & sValue) {
    BatchPaxosValues oBatchValues;
    bool is_succeeded = oBatchValues.ParseFromArray(sValue.data() + sizeof(int), sValue.size() - sizeof(int));
    if (!is_succeeded) {
        return;
    }

    bool change = false;
    std::set<int> mapStateMachineAlreadyCall;
    for (int i = 0; i < oBatchValues.values_size(); i ++) {
        PaxosValue * poValue = oBatchValues.mutable_values(i);

        if (mapStateMachineAlreadyCall.find(poValue->smid()) != end(mapStateMachineAlreadyCall)) {
            continue;
        }
        mapStateMachineAlreadyCall.insert(poValue->smid());

        BeforeProposeCall(group_idx, poValue->smid(), *poValue->mutable_value(), change);
    }

    if (change) {
        std::string sBodyValue;
        is_succeeded = oBatchValues.SerializeToString(&sBodyValue);
        assert(is_succeeded == true);
        sValue.erase(sizeof(int));
        sValue.append(sBodyValue);
    }
}

void StateMachineFac::BeforeProposeCall(const int group_idx, const int state_machine_id, std::string & sBodyValue, bool & change) {
    if (state_machine_id == 0) {
        return;
    }

    if (vec_state_machine_list_.size() == 0) {
        return;
    }

    for (auto & state_machine : vec_state_machine_list_) {
        if (state_machine->StateMachineId() == state_machine_id) {
            if (state_machine->NeedCallBeforePropose()) {
                change  = true;
                return state_machine->BeforePropose(group_idx, sBodyValue);
            }
        }
    }
}

}



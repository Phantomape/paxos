#include "system_variables_store.h"
#include "db.h"

namespace paxos {

SystemVariablesStore::SystemVariablesStore(const LogStorage * poLogStorage) : log_storage_((LogStorage *)poLogStorage) {}

SystemVariablesStore::~SystemVariablesStore() {}

int SystemVariablesStore::Write(const WriteOptions & write_options, const int group_idx, const SystemVariables & system_variables) {
    const int m_iMyGroupIdx = group_idx;

    std::string buffer;
    bool sSucc = system_variables.SerializeToString(&buffer);
    if (!sSucc) {
        return -1;
    }
    
    int ret = log_storage_->SetSystemVariables(write_options, group_idx, buffer);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

int SystemVariablesStore::Read(const int group_idx, SystemVariables & system_variables) {
    const int m_iMyGroupIdx = group_idx;

    std::string buffer;
    int ret = log_storage_->GetSystemVariables(group_idx, buffer);
    if (ret != 0 && ret != 1) {
        return ret;
    }
    else if (ret == 1) {
        return 1;
    }

    bool is_succeeded = system_variables.ParseFromArray(buffer.data(), buffer.size());
    if (!is_succeeded) {
        return -1;
    }

    return 0;
}

}



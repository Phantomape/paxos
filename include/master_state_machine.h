#pragma once

#include <mutex>
#include "state_machine.h"
#include "def.h"
#include "internal_state_machine.h"
#include "master_sm.pb.h"
#include "master_state_machine.h"
#include "master_variables_store.h"
#include "options.h"

namespace paxos {

enum MasterOperatorType
{
    MasterOperatorType_Complete = 1,
};

class MasterStateMachine : public InternalStateMachine 
{
public:
    MasterStateMachine(
        const LogStorage * poLogStorage, 
        const uint64_t iMyNodeID, 
        const int iGroupIdx,
        MasterChangeCallback pMasterChangeCallback);
    ~MasterStateMachine();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, const std::string & sValue, StateMachineCtx * poSMCtx);

    const int StateMachineId() const;

    bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue)
    {
        return true;
    }

    const uint64_t GetCheckpointInstanceID(const int iGroupIdx) const
    {
        return m_llMasterVersion;
    }

    void BeforePropose(const int iGroupIdx, std::string & sValue);

    const bool NeedCallBeforePropose();

public:
    int GetCheckpointState(const int iGroupIdx, std::string & sDirPath, 
            std::vector<std::string> & vecFileList)
    {
        return 0;
    }    
    
    int LoadCheckpointState(const int iGroupIdx, const std::string & sCheckpointTmpFileDirPath,
            const std::vector<std::string> & vecFileList, const uint64_t llCheckpointInstanceID)
    {
        return 0;
    }

    int LockCheckpointState()
    {
        return 0;
    }

    void UnLockCheckpointState()
    {
    }

public:
    int Init();

    int LearnMaster(
            const uint64_t llInstanceID,
            const MasterOperator & oMasterOper, 
            const uint64_t llAbsMasterTimeout = 0);

    const uint64_t GetMaster() const;

    const uint64_t GetMasterWithVersion(uint64_t & llVersion);

    const bool IsIMMaster() const;

public:
    int UpdateMasterToStore(const uint64_t llMasterNodeID, const uint64_t llVersion, const uint32_t iLeaseTime);

    void SafeGetMaster(uint64_t & iMasterNodeID, uint64_t & llMasterVersion);

public:
    static bool MakeOpValue(
            const uint64_t iNodeID, 
            const uint64_t llVersion,
            const int iTimeout,
            const MasterOperatorType iOp,    
            std::string & sPaxosValue);

public:
    int GetCheckpointBuffer(std::string & sCPBuffer);

    int UpdateByCheckpoint(const std::string & sCPBuffer, bool & bChange);

private:
    int m_iMyGroupIdx;
    uint64_t m_iMyNodeID;

private:
    MasterVariablesStore m_oMVStore;
    
    uint64_t m_iMasterNodeID;
    uint64_t m_llMasterVersion;
    int m_iLeaseTime;
    uint64_t m_llAbsExpireTime;

    std::mutex m_oMutex;

    MasterChangeCallback m_pMasterChangeCallback;
};
    
}

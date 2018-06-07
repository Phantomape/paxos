#pragma once

#include "node.h"
#include "def.h"
#include "rpc.pb.h"
#include "state_machine.h"
#include "client.h"
#include <string>
#include <vector>

#include <grpc++/grpc++.h>

class PhxEchoSMCtx {
public:
    int iExecuteRet;
    std::string sEchoRespValue;

    PhxEchoSMCtx() {
        iExecuteRet = -1;
    }
};

class PhxEchoSM : public paxos::StateMachine {
public:
    PhxEchoSM();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, paxos::StateMachineCtx * poSMCtx);

    const int StateMachineId() const { return 1; }
};

class PhxEchoServer {
public:
    PhxEchoServer(const paxos::NodeInfo & oMyNode, const paxos::NodeInfoList & vecNodeList);

    ~PhxEchoServer();

    int Run();

    int Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue);

private:
    int MakeLogStoragePath(std::string & sLogStoragePath);

    paxos::NodeInfo node_info_;

    paxos::NodeInfoList vec_node_list_;

    paxos::Node* node_;
    
    PhxEchoSM echo_state_machine_;
};

// I still have no idea what ctx does
class KVStateMachineCtx {
public:
    int execute_ret_;
    std::string read_val_;
    uint64_t read_version_;

    KVStateMachineCtx() {
        execute_ret_ = -1;
        read_version_ = 0;
    }
};

class KVStateMachine : public paxos::StateMachine {
public:
    KVStateMachine(const std::string & sDBPath);

    ~KVStateMachine();

    const bool Init();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, paxos::StateMachineCtx * poSMCtx);

    const int SMID() const {return 1;}

    //no use
    bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue) {return true;}

    //have checkpoint.
    const uint64_t GetCheckpointInstanceID(const int iGroupIdx) const { return m_llCheckpointInstanceID;}

    //have checkpoint, but not impl auto copy checkpoint to other node, so return fail.
    int LockCheckpointState() { return -1; }
    
    int GetCheckpointState(const int iGroupIdx, std::string & sDirPath, 
            std::vector<std::string> & vecFileList) { return -1; }

    void UnLockCheckpointState() { }
    
    int LoadCheckpointState(const int iGroupIdx, const std::string & sCheckpointTmpFileDirPath,
            const std::vector<std::string> & vecFileList, const uint64_t llCheckpointInstanceID) { return -1; }

    static bool MakeOpValue(
            const std::string & sKey, 
            const std::string & sValue, 
            const uint64_t llVersion, 
            const paxos::KVOperatorType iOp,
            std::string & sPaxosValue);

    static bool MakeGetOpValue(
            const std::string & sKey,
            std::string & sPaxosValue);

    static bool MakeSetOpValue(
            const std::string & sKey, 
            const std::string & sValue, 
            const uint64_t llVersion, 
            std::string & sPaxosValue);

    static bool MakeDelOpValue(
            const std::string & sKey, 
            const uint64_t llVersion, 
            std::string & sPaxosValue);

    KVClient * GetKVClient();

    int SyncCheckpointInstanceID(const uint64_t llInstanceID);

private:
    std::string m_sDBPath;
    KVClient m_oKVClient;

    uint64_t m_llCheckpointInstanceID;
    int m_iSkipSyncCheckpointTimes;
};

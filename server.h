#pragma once

#include "node.h"
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

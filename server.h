#pragma once

#include "node.h"
#include <string>
#include <vector>

class PhxEchoSMCtx
{
public:
    int iExecuteRet;
    std::string sEchoRespValue;

    PhxEchoSMCtx()
    {
        iExecuteRet = -1;
    }
};

class PhxEchoSM : public paxos::StateMachine
{
public:
    PhxEchoSM();

    bool Execute(const int iGroupIdx, const uint64_t llInstanceID, 
            const std::string & sPaxosValue, paxos::StateMachineCtx * poSMCtx);

    const int StateMachineId() const { return 1; }
};

class PhxEchoServer
{
public:
    PhxEchoServer(const paxos::NodeInfo & oMyNode, const paxos::NodeInfoList & vecNodeList);

    ~PhxEchoServer();

    int Run();

    int Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue);

private:
    int MakeLogStoragePath(std::string & sLogStoragePath);

private:
    paxos::NodeInfo m_oMyNode;

    paxos::NodeInfoList m_vecNodeList;

    paxos::Node * m_poPaxosNode;
    
    PhxEchoSM m_oEchoSM;
};
    

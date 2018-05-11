#include "server.h"
#include "options.h"
#include <assert.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace paxos;

PhxEchoSM::PhxEchoSM()
{
}

bool PhxEchoSM::Execute(const int iGroupIdx, const uint64_t llInstanceID, 
        const std::string & sPaxosValue, paxos::StateMachineCtx * poSMCtx)
{
    //only commiter node have SMCtx.
    if (poSMCtx != nullptr && poSMCtx->ctx_ != nullptr)
    {
        PhxEchoSMCtx * poPhxEchoSMCtx = (PhxEchoSMCtx *)poSMCtx->ctx_;
        poPhxEchoSMCtx->iExecuteRet = 0;
        poPhxEchoSMCtx->sEchoRespValue = sPaxosValue;
    }

    return true;
}

PhxEchoServer::PhxEchoServer(const paxos::NodeInfo & oMyNode, const paxos::NodeInfoList & vecNodeList)
    : m_oMyNode(oMyNode), m_vecNodeList(vecNodeList), m_poPaxosNode(nullptr)
{
}

PhxEchoServer::~PhxEchoServer()
{
    delete m_poPaxosNode;
}

int PhxEchoServer::MakeLogStoragePath(std::string & sLogStoragePath)
{
    char sTmp[128] = {0};

    sLogStoragePath = string(sTmp);

    if (access(sLogStoragePath.c_str(), F_OK) == -1)
    {
        if (mkdir(sLogStoragePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
        {       
            printf("Create dir fail, path %s\n", sLogStoragePath.c_str());
            return -1;
        }       
    }

    return 0;
}

int PhxEchoServer::Run()
{
    paxos::Options oOptions;

    int ret = MakeLogStoragePath(oOptions.log_storage_path_);
    if (ret != 0)
    {
        return ret;
    }

    //this groupcount means run paxos group count.
    //every paxos group is independent, there are no any communicate between any 2 paxos group.
    oOptions.group_count_ = 1;

    oOptions.node_ = m_oMyNode;
    oOptions.vec_node_info_list_ = m_vecNodeList;

    GroupStateMachineInfo oSMInfo;
    oSMInfo.group_idx_ = 0;
    //one paxos group can have multi state machine.
    oSMInfo.vec_state_machine_list_.push_back(&m_oEchoSM);
    oOptions.vec_group_state_machine_info_list_.push_back(oSMInfo);

    //use logger_google to print log
    LogFunc log_func;

    //set logger
    oOptions.log_func_ = log_func;

    ret = Node::Run(oOptions, m_poPaxosNode);
    if (ret != 0)
    {
        printf("run paxos fail, ret %d\n", ret);
        return ret;
    }

    printf("run paxos ok\n");
    return 0;
}

int PhxEchoServer::Echo(const std::string & sEchoReqValue, std::string & sEchoRespValue)
{
    StateMachineCtx oCtx;
    PhxEchoSMCtx oEchoSMCtx;
    //smid must same to PhxEchoSM.SMID().
    oCtx.state_machine_id_ = 1;
    oCtx.ctx_ = (void *)&oEchoSMCtx;

    uint64_t llInstanceID = 0;
    int ret = m_poPaxosNode->Propose(0, sEchoReqValue, llInstanceID, &oCtx);
    if (ret != 0)
    {
        printf("paxos propose fail, ret %d\n", ret);
        return ret;
    }

    if (oEchoSMCtx.iExecuteRet != 0)
    {
        printf("echo sm excute fail, excuteret %d\n", oEchoSMCtx.iExecuteRet);
        return oEchoSMCtx.iExecuteRet;
    }

    sEchoRespValue = oEchoSMCtx.sEchoRespValue.c_str();

    return 0;
}
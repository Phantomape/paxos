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

PhxEchoSM::PhxEchoSM() {
}

bool PhxEchoSM::Execute(
    const int group_idx, 
    const uint64_t instance_id, 
    const std::string & sPaxosValue, 
    paxos::StateMachineCtx * poSMCtx
    ) {
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
    : node_info_(oMyNode), vec_node_list_(vecNodeList), node_(nullptr) {
}

PhxEchoServer::~PhxEchoServer()
{
    delete node_;
}

int PhxEchoServer::MakeLogStoragePath(std::string & sLogStoragePath) {
    char sTmp[128] = {0};
    snprintf(sTmp, sizeof(sTmp), "./logpath_%s_%d", node_info_.GetIp().c_str(), node_info_.GetPort());

    sLogStoragePath = string(sTmp);

    if (access(sLogStoragePath.c_str(), F_OK) == -1) {
        if (mkdir(sLogStoragePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {       
            printf("Create dir fail, path %s\n", sLogStoragePath.c_str());
            return -1;
        }       
    }

    return 0;
}

int PhxEchoServer::Run() {
    paxos::Options options;

    int ret = MakeLogStoragePath(options.log_storage_path_);
    if (ret != 0) {
        return ret;
    }

    //this groupcount means run paxos group count.
    //every paxos group is independent, there are no any communicate between any 2 paxos group.
    options.group_count_ = 1;
    options.node_ = node_info_;
    options.vec_node_info_list_ = vec_node_list_;

    GroupStateMachineInfo state_machine_info;
    state_machine_info.group_idx_ = 0;
    //one paxos group can have multi state machine.
    state_machine_info.vec_state_machine_list_.push_back(&echo_state_machine_);
    options.vec_group_state_machine_info_list_.push_back(state_machine_info);

    //use logger_google to print log
    LogFunc log_func;
    options.log_func_ = log_func;

    ret = Node::Run(options, node_);
    if (ret != 0) {
        printf("run paxos fail, ret %d\n", ret);
        return ret;
    }

    printf("run paxos ok\n");
    return 0;
}

int PhxEchoServer::Echo(const std::string& echo_send_val, std::string& sEchoRespValue) {
    StateMachineCtx oCtx;
    PhxEchoSMCtx oEchoSMCtx;
    //smid must same to PhxEchoSM.SMID().
    oCtx.state_machine_id_ = 1;
    oCtx.ctx_ = (void *)&oEchoSMCtx;

    uint64_t instance_id = 0;
    int ret = node_->Propose(0, echo_send_val, instance_id, &oCtx);
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
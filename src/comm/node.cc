#include "node.h"
#include "pnode.h"

namespace paxos {

int Node::Run(const Options & options, Node *& node) {
    if (options.is_large_value_mode_) {
        InternalOptions::Instance()->SetAsLargeBufferMode();
    }
    
    InternalOptions::Instance()->SetGroupCount(options.group_count_);
        
    node = nullptr;
    Network* network = nullptr;

    //Breakpoint::m_poBreakpoint = nullptr;
    //BP->SetInstance(options.poBreakpoint);
    
    PNode * pnode = new PNode();
    int ret = pnode->Init(options, network);
    if (ret != 0) {
        delete pnode;
        return ret;
    }

    //step1 set node to network
    //very important, let network on recieve callback can work.
    network->node = pnode;

    //step2 run network.
    //start recieve message from network, so all must init before this step.
    //must be the last step.
    network->Run();

    node = pnode;
    
    return 0;
}
    
}



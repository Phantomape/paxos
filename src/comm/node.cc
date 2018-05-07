#include "node.h"
#include "pnode.h"

namespace paxos {

int Node::Run(const Options & oOptions, Node *& poNode)
{
    if (oOptions.is_large_value_mode_)
    {
        InternalOptions::Instance()->SetAsLargeBufferMode();
    }
    
    InternalOptions::Instance()->SetGroupCount(oOptions.group_count_);
        
    poNode = nullptr;
    Network* network = nullptr;

    //Breakpoint::m_poBreakpoint = nullptr;
    //BP->SetInstance(oOptions.poBreakpoint);

    /*
    PNode * poRealNode = new PNode();
    int ret = poRealNode->Init(oOptions, network);
    if (ret != 0)
    {
        delete poRealNode;
        return ret;
    }

    //step1 set node to network
    //very important, let network on recieve callback can work.
    network->m_poNode = poRealNode;

    //step2 run network.
    //start recieve message from network, so all must init before this step.
    //must be the last step.
    network->Run();

    poNode = poRealNode;
    */
   
    return 0;
}
    
}



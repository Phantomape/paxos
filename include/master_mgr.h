#pragma once

#include "concurrent.h"
#include "node.h"
#include "master_state_machine.h"

namespace paxos {

class MasterMgr : public Thread {
public:
    MasterMgr(const Node * poPaxosNode, 
        const int iGroupIdx, 
        const LogStorage * poLogStorage,
        MasterChangeCallback pMasterChangeCallback);

    ~MasterMgr();

    void RunMaster();

    void StopMaster();

    int Init();

    void Run();

    void SetLeaseTime(const int iLeaseTimeMs);

    void TryBeMaster(const int iLeaseTime);

    void DropMaster();

    MasterStateMachine * GetMasterSM();

private:
    Node * m_poPaxosNode;

    MasterStateMachine m_oDefaultMasterSM;

    int m_iLeaseTime;

    bool m_bIsEnd;
    bool m_bIsStarted;

    int m_iMyGroupIdx;

    bool m_bNeedDropMaster;
};

}

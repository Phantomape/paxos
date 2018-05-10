#pragma once

#include "committer.h"
#include "instance.h"
#include "cleaner.h"
#include "communicator.h"
#include "options.h"
#include "network.h"
#include "replayer.h"
#include <thread>

namespace paxos {

class Group
{
public:
    Group(LogStorage * poLogStorage, 
            Network * poNetwork,    
            InternalStateMachine * poMasterSM,
            const int iGroupIdx,
            const Options & oOptions);

    ~Group();

    void StartInit();

    void Init();

    int GetInitRet();

    void Start();

    void Stop();

    Config * GetConfig();

    Instance * GetInstance();

    Committer * GetCommitter();

    Cleaner * GetCheckpointCleaner();

    Replayer * GetCheckpointReplayer();

    void AddStateMachine(StateMachine * poSM);

private:
    Communicator m_oCommunicate;
    Config m_oConfig;
    Instance m_oInstance;

    int m_iInitRet;
    std::thread * m_poThread;
};

}

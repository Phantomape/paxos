#include "group.h"

namespace paxos {

Group::Group(LogStorage * poLogStorage, 
            Network * poNetwork,
            InternalStateMachine * poMasterSM,
            const int iGroupIdx,
            const Options & oOptions) : 
    m_oCommunicate(&m_oConfig, oOptions.node_.GetNodeId(), oOptions.udp_max_msg_size_, poNetwork),
    m_oConfig(poLogStorage, oOptions.sync_, oOptions.sync_interval_, oOptions.use_membership_, 
            oOptions.node_, oOptions.vec_node_info_list_, oOptions.vec_follower_node_info_list_, 
            iGroupIdx, oOptions.group_count_, oOptions.membership_change_callback_),
    m_oInstance(&m_oConfig, poLogStorage, &m_oCommunicate, oOptions),
    m_iInitRet(-1), m_poThread(nullptr) {
    m_oConfig.SetMasterSM(poMasterSM);
}

Group::~Group() {
}

void Group::StartInit() {
    m_poThread = new std::thread(&Group::Init, this);
    assert(m_poThread != nullptr);
}

void Group::Init() {
    m_iInitRet = m_oConfig.Init();
    if (m_iInitRet != 0) {
        return;
    }

    //inside sm
    AddStateMachine(m_oConfig.GetSystemVSM());
    AddStateMachine(m_oConfig.GetMasterSM());

    m_iInitRet = m_oInstance.Init();
}

int Group::GetInitRet() {
    m_poThread->join();
    delete m_poThread;

    return m_iInitRet;
}

void Group::Start() {
    m_oInstance.Start();
}

void Group::Stop() {
    m_oInstance.Stop();
}

Config * Group::GetConfig() {
    return &m_oConfig;
}

Instance * Group::GetInstance() {
    return &m_oInstance;
}

Committer * Group::GetCommitter() {
    return m_oInstance.GetCommitter();
}

Cleaner * Group::GetCheckpointCleaner() {
    return m_oInstance.GetCheckpointCleaner();
}

Replayer * Group::GetCheckpointReplayer() {
    return m_oInstance.GetCheckpointReplayer();
}

void Group::AddStateMachine(StateMachine * poSM) {
    m_oInstance.AddStateMachine(poSM);
}

}

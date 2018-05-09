#include "config.h"
#include "internal_options.h"
#include "inttypes.h"
#include "util.h"
#include <math.h>

namespace paxos {

Config::Config(
        const LogStorage * poLogStorage,
        const bool bLogSync,
        const int iSyncInterval,
        const bool bUseMembership,
        const NodeInfo & oMyNode, 
        const NodeInfoList & vecNodeInfoList,
        const FollowerNodeInfoList & vecFollowerNodeInfoList,
        const int iMyGroupIdx,
        const int iGroupCount,
        MembershipChangeCallback pMembershipChangeCallback)
    : m_bLogSync(bLogSync), 
    m_iSyncInterval(iSyncInterval),
    m_bUseMembership(bUseMembership),
    m_iMyNodeID(oMyNode.GetNodeId()), 
    m_iNodeCount(vecNodeInfoList.size()), 
    m_iMyGroupIdx(iMyGroupIdx),
    m_iGroupCount(iGroupCount),
    m_oSystemVSM(iMyGroupIdx, oMyNode.GetNodeId(), poLogStorage, pMembershipChangeCallback),
    m_poMasterSM(nullptr) {
    m_vecNodeInfoList = vecNodeInfoList;

    m_bIsIMFollower = false;
    m_iFollowToNodeID = nullnode;

    for (auto & oFollowerNodeInfo : vecFollowerNodeInfoList) {
        if (oFollowerNodeInfo.node_.GetNodeId() == oMyNode.GetNodeId()) {
            m_bIsIMFollower = true;
            m_iFollowToNodeID = oFollowerNodeInfo.node_.GetNodeId();

            InternalOptions::Instance()->SetAsFollower();
        }
    }
}

Config::~Config()
{
}

int Config::Init() {
    int ret = m_oSystemVSM.Init();
    if (ret != 0) {
        return ret;
    }

    m_oSystemVSM.AddNodeIDList(m_vecNodeInfoList);

    return 0;
}

const bool Config::CheckConfig() {
    if (!m_oSystemVSM.IsIMInMembership()) {
        return false;
    }

    return true;
}

const uint64_t Config::GetGid() const {
    return m_oSystemVSM.GetGid();
}

const uint64_t Config::GetMyNodeID() const {
    return m_iMyNodeID;
}

const int Config::GetNodeCount() const {
    return m_oSystemVSM.GetNodeCount();
}

const int Config::GetMyGroupIdx() const {
    return m_iMyGroupIdx;
}

const int Config::GetGroupCount() const {
    return m_iGroupCount;
}

const int Config::GetMajorityCount() const {
    return m_oSystemVSM.GetMajorityCount();
}

const bool Config::GetIsUseMembership() const {
    return m_bUseMembership;
}

const uint64_t Config::GetAskforLearnTimeoutMs() const {
    return 2000;
}

const int Config::GetPrepareTimeoutMs() const {
    return 3000;
}

const int Config::GetAcceptTimeoutMs() const {
    return 3000;
}

const bool Config::IsValidNodeID(const uint64_t iNodeId) {
    return m_oSystemVSM.IsValidNodeID(iNodeId);
}

const bool Config::IsIMFollower() const {
    return m_bIsIMFollower;
}

const uint64_t Config::GetFollowToNodeID() const {
    return m_iFollowToNodeID;
}

SystemVSM * Config::GetSystemVSM() {
    return &m_oSystemVSM;
}

void Config::SetMasterSM(InternalStateMachine * poMasterSM) {
    m_poMasterSM = poMasterSM;
}

InternalStateMachine * Config::GetMasterSM() {
    return m_poMasterSM;
}

#define TmpNodeTimeout 60000

void Config::AddTmpNodeOnlyForLearn(const uint64_t iTmpNodeId) {
    const std::set<uint64_t> & setNodeId = m_oSystemVSM.GetMembershipMap();
    if (setNodeId.find(iTmpNodeId) != end(setNodeId)) {
        return;
    }

    m_mapTmpNodeOnlyForLearn[iTmpNodeId] = Time::GetSteadyClockMS() + TmpNodeTimeout;
}

const std::map<uint64_t, uint64_t> & Config::GetTmpNodeMap() {
    uint64_t llNowTime = Time::GetSteadyClockMS();

    for (auto it = m_mapTmpNodeOnlyForLearn.begin(); it != end(m_mapTmpNodeOnlyForLearn);) {
        if (it->second < llNowTime) {
            it = m_mapTmpNodeOnlyForLearn.erase(it);
        }
        else {
            it++;
        }
    }

    return m_mapTmpNodeOnlyForLearn;
}

void Config::AddFollowerNode(const uint64_t iMyFollowerNodeId) {
    static int iFollowerTimeout = ASKFORLEARN_NOOP_INTERVAL * 3;
    m_mapMyFollower[iMyFollowerNodeId] = Time::GetSteadyClockMS() + iFollowerTimeout;
}

const std::map<uint64_t, uint64_t> & Config::GetMyFollowerMap() {
    uint64_t llNowTime = Time::GetSteadyClockMS();

    for (auto it = m_mapMyFollower.begin(); it != end(m_mapMyFollower);) {
        if (it->second < llNowTime) {
            it = m_mapMyFollower.erase(it);
        }
        else {
            it++;
        }
    }

    return m_mapMyFollower;
}

const size_t Config::GetMyFollowerCount() {
    return m_mapMyFollower.size();
}

const bool Config::LogSync() const {
    return m_bLogSync;
}

void Config::SetLogSync(const bool LogSync) {
    m_bLogSync = LogSync;
}

const int Config::SyncInterval() const {
    return m_iSyncInterval;
}

}



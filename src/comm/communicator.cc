#include "communicator.h"
#include "internal_options.h"
#include "def.h"

namespace paxos
{

Communicator::Communicator(
        const Config * poConfig,
        const uint64_t iMyNodeID, 
        const int iUDPMaxSize,
        Network * poNetwork)
    : config_((Config *)poConfig), network_(poNetwork), node_id_(iMyNodeID), udp_max_size_(iUDPMaxSize)
{
}

Communicator::~Communicator()
{
}

int Communicator::Send(const int iGroupIdx, const uint64_t iNodeID, 
        const NodeInfo & oNodeInfo, const std::string & sMessage, const int iSendType)
{
    if ((int)sMessage.size() > MAX_VALUE_SIZE)
    {
        //BP->GetNetworkBP()->SendRejectByTooLargeSize();
        //PLGErr("Message size too large %zu, max size %u, skip message", 
        //        sMessage.size(), MAX_VALUE_SIZE);
        return 0;
    }

    if (sMessage.size() > udp_max_size_ || iSendType == Message_SendType_TCP) {
        //BP->GetNetworkBP()->SendTcp(sMessage);
        return network_->SendMessageTCP(iGroupIdx, oNodeInfo.GetIp(), oNodeInfo.GetPort(), sMessage);
    }
    else {
        //BP->GetNetworkBP()->SendUdp(sMessage);
        return network_->SendMessageUDP(iGroupIdx, oNodeInfo.GetIp(), oNodeInfo.GetPort(), sMessage);
    }
}

int Communicator::SendMessage(const int iGroupIdx, const uint64_t iSendtoNodeID, const std::string & sMessage, const int iSendType) {
    return Send(iGroupIdx, iSendtoNodeID, NodeInfo(iSendtoNodeID), sMessage, iSendType);
}

int Communicator::BroadcastMessage(const int iGroupIdx, const std::string & sMessage, const int iSendType) {
    const std::set<uint64_t> & setNodeInfo = config_->GetSystemVSM()->GetMembershipMap();

    for (auto & it : setNodeInfo) {
        if (it != node_id_) {
            Send(iGroupIdx, it, NodeInfo(it), sMessage, iSendType);
        }
    }

    return 0;
}

int Communicator::BroadcastMessageFollower(const int iGroupIdx, const std::string & sMessage, const int iSendType) {
    const std::map<uint64_t, uint64_t> & mapFollowerNodeInfo = config_->GetMyFollowerMap();

    for (auto & it : mapFollowerNodeInfo) {
        if (it.first != node_id_) {
            Send(iGroupIdx, it.first, NodeInfo(it.first), sMessage, iSendType);
        }
    }

    return 0;
}

int Communicator::BroadcastMessageTempNode(const int iGroupIdx, const std::string & sMessage, const int iSendType) {
    const std::map<uint64_t, uint64_t> & mapTempNode = config_->GetTmpNodeMap();

    for (auto & it : mapTempNode) {
        if (it.first != node_id_) {
            Send(iGroupIdx, it.first, NodeInfo(it.first), sMessage, iSendType);
        }
    }

    return 0;
}

void Communicator::SetUDPMaxSize(const size_t iUDPMaxSize) {
    udp_max_size_ = iUDPMaxSize;
}

}



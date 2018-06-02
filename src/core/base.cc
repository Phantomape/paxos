#include "base.h"
#include "def.h"
#include "communicate.h"
#include "instance.h"
#include "crc32.h"
#include <iostream>
#include <string>

namespace paxos {

Base::Base(const Config * poConfig, const Communicate * poCommunicate, const Instance * poInstance)
{
    config_ = (Config *)poConfig;
    communicate_ = (Communicate *)poCommunicate;
    instance_ = (Instance *)poInstance;

    instance_id_ = 0;

    is_test_mode_ = false;
}

Base::~Base()
{
}

uint64_t Base::GetInstanceId()
{
    return instance_id_;
}

void Base::SetInstanceId(const uint64_t llInstanceId)
{
    instance_id_ = llInstanceId;
}

void Base::NewInstance()
{
    instance_id_++;
    InitInstance();
}

const uint32_t Base::GetLastChecksum() const
{
    //return instance_->GetLastChecksum();
}

int Base::PackMsg(const PaxosMsg & oPaxosMsg, std::string & sBuffer)
{
    std::string sBodyBuffer;
    bool bSucc = oPaxosMsg.SerializeToString(&sBodyBuffer);
    if (!bSucc)
    {
        //PLGErr("PaxosMsg.SerializeToString fail, skip this msg");
        return -1;
    }

    int iCmd = MsgCmd_PaxosMsg;
    PackBaseMsg(sBodyBuffer, iCmd, sBuffer);

    return 0;
}

int Base::PackCheckpointMsg(const CheckpointMsg & oCheckpointMsg, std::string & sBuffer)
{
    std::string sBodyBuffer;
    bool bSucc = oCheckpointMsg.SerializeToString(&sBodyBuffer);
    if (!bSucc)
    {
        //PLGErr("CheckpointMsg.SerializeToString fail, skip this msg");
        return -1;
    }

    int iCmd = MsgCmd_CheckpointMsg;
    PackBaseMsg(sBodyBuffer, iCmd, sBuffer);

    return 0;
}

void Base::PackBaseMsg(const std::string & sBodyBuffer, const int iCmd, std::string & sBuffer)
{
    char sGroupIdx[GROUPIDXLEN] = {0};
    int iGroupIdx = config_->GetMyGroupIdx();
    memcpy(sGroupIdx, &iGroupIdx, sizeof(sGroupIdx));

    Header oHeader;
    oHeader.set_gid(config_->GetGid());
    oHeader.set_rid(0);
    oHeader.set_cmdid(iCmd);
    oHeader.set_version(1);

    std::string sHeaderBuffer;
    bool bSucc = oHeader.SerializeToString(&sHeaderBuffer);
    if (!bSucc)
    {
        //PLGErr("Header.SerializeToString fail, skip this msg");
        assert(bSucc == true);
    }

    char sHeaderLen[HEADLEN_LEN] = {0};
    uint16_t iHeaderLen = (uint16_t)sHeaderBuffer.size();
    memcpy(sHeaderLen, &iHeaderLen, sizeof(sHeaderLen));

    sBuffer = std::string(sGroupIdx, sizeof(sGroupIdx)) + std::string(sHeaderLen, sizeof(sHeaderLen)) + sHeaderBuffer + sBodyBuffer;

    //check sum
    uint32_t iBufferChecksum = crc32(0, (const uint8_t *)sBuffer.data(), sBuffer.size(), NET_CRC32SKIP);
    char sBufferChecksum[CHECKSUM_LEN] = {0};
    memcpy(sBufferChecksum, &iBufferChecksum, sizeof(sBufferChecksum));

    sBuffer += std::string(sBufferChecksum, sizeof(sBufferChecksum));
}

int Base::UnPackBaseMsg(const std::string & sBuffer, Header & oHeader, size_t & iBodyStartPos, size_t & iBodyLen)
{
    uint16_t iHeaderLen = 0;
    memcpy(&iHeaderLen, sBuffer.data() + GROUPIDXLEN, HEADLEN_LEN);

    size_t iHeaderStartPos = GROUPIDXLEN + HEADLEN_LEN;
    iBodyStartPos = iHeaderStartPos + iHeaderLen;

    if (iBodyStartPos > sBuffer.size())
    {
        //BP->GetAlgorithmBaseBP()->UnPackHeaderLenTooLong();
        //NLErr("Header headerlen too loog %d", iHeaderLen);
        return -1;
    }

    bool bSucc = oHeader.ParseFromArray(sBuffer.data() + iHeaderStartPos, iHeaderLen);
    if (!bSucc)
    {
        //NLErr("Header.ParseFromArray fail, skip this msg");
        return -1;
    }

    //NLDebug("buffer_size %zu header len %d cmdid %d gid %lu rid %lu version %d body_startpos %zu",
    //        sBuffer.size(), iHeaderLen, oHeader.cmdid(), oHeader.gid(), oHeader.rid(), oHeader.version(), iBodyStartPos);

    if (oHeader.version() >= 1)
    {
        if (iBodyStartPos + CHECKSUM_LEN > sBuffer.size())
        {
            //NLErr("no checksum, body start pos %zu buffersize %zu", iBodyStartPos, sBuffer.size());
            return -1;
        }

        iBodyLen = sBuffer.size() - CHECKSUM_LEN - iBodyStartPos;

        uint32_t iBufferChecksum = 0;
        memcpy(&iBufferChecksum, sBuffer.data() + sBuffer.size() - CHECKSUM_LEN, CHECKSUM_LEN);

        uint32_t iNewCalBufferChecksum = crc32(0, (const uint8_t *)sBuffer.data(), sBuffer.size() - CHECKSUM_LEN, NET_CRC32SKIP);
        if (iNewCalBufferChecksum != iBufferChecksum)
        {
            //BP->GetAlgorithmBaseBP()->UnPackChecksumNotSame();
            return -1;
        }

        /*
        NLDebug("Checksum compare ok, Data.bring.checksum %u, Data.cal.checksum %u",
                iBufferChecksum, iNewCalBufferChecksum) 
        */
    }
    else
    {
        iBodyLen = sBuffer.size() - iBodyStartPos;
    }

    return 0;
}

int Base::SendMessage(const uint64_t iSendtoNodeId, const CheckpointMsg & oCheckpointMsg, const int iSendType) {
    if (iSendtoNodeId == config_->GetMyNodeID()) {
        return 0;
    }
    std::string sBuffer;
    int ret = PackCheckpointMsg(oCheckpointMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return communicate_->SendMessage(config_->GetMyGroupIdx(), iSendtoNodeId, sBuffer, iSendType);
}

int Base::SendMessage(const uint64_t iSendtoNodeId, const PaxosMsg & oPaxosMsg, const int iSendType)
{
    if (is_test_mode_)
    {
        return 0;
    }

    //BP->GetInstanceBP()->SendMessage();

    if (iSendtoNodeId == config_->GetMyNodeID())
    {
        instance_->OnReceivePaxosMsg(oPaxosMsg);
        return 0;
    }

    std::string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return communicate_->SendMessage(config_->GetMyGroupIdx(), iSendtoNodeId, sBuffer, iSendType);
}

int Base::BroadcastMessage(const PaxosMsg & oPaxosMsg, const int iRunType, const int iSendType)
{
    if (is_test_mode_)
    {
        return 0;
    }

    //BP->GetInstanceBP()->BroadcastMessage();

    if (iRunType == BroadcastMessage_Type_RunSelf_First)
    {
        if (instance_->OnReceivePaxosMsg(oPaxosMsg) != 0)
        {
            return -1;
        }
    }

    std::string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    ret = communicate_->BroadcastMessage(config_->GetMyGroupIdx(), sBuffer, iSendType);

    if (iRunType == BroadcastMessage_Type_RunSelf_Final)
    {
        instance_->OnReceivePaxosMsg(oPaxosMsg);
    }

    return ret;
}

int Base::BroadcastMessageToFollower(const PaxosMsg & oPaxosMsg, const int iSendType)
{
    std::string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return communicate_->BroadcastMessageFollower(config_->GetMyGroupIdx(), sBuffer, iSendType);
}

int Base::BroadcastMessageToTempNode(const PaxosMsg & oPaxosMsg, const int iSendType)
{
    std::string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    return communicate_->BroadcastMessageTempNode(config_->GetMyGroupIdx(), sBuffer, iSendType);
}

///////////////////////////

void Base::SetAsTestMode()
{
    is_test_mode_ = true;
}

}


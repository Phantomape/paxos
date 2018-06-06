#include "base.h"
#include "def.h"
#include "communicate.h"
#include "instance.h"
#include "crc32.h"
#include <iostream>
#include <string>

namespace paxos {

Base::Base(const Config * poConfig, const Communicate * poCommunicate, const Instance * poInstance) {
    config_ = (Config *)poConfig;
    communicate_ = (Communicate *)poCommunicate;
    instance_ = (Instance *)poInstance;

    instance_id_ = 0;

    is_test_mode_ = false;
}

Base::~Base() {
}

uint64_t Base::GetInstanceId() {
    return instance_id_;
}

void Base::SetInstanceId(const uint64_t llInstanceId) {
    instance_id_ = llInstanceId;
}

void Base::NewInstance() {
    instance_id_++;
    InitInstance();
}

const uint32_t Base::GetLastChecksum() const {
    return instance_->GetLastChecksum();
}

int Base::PackMsg(const PaxosMsg & paxos_msg, std::string & sBuffer) {
    std::string sBodyBuffer;
    bool is_succeeded = paxos_msg.SerializeToString(&sBodyBuffer);
    if (!is_succeeded) {
        return -1;
    }

    int iCmd = MsgCmd_PaxosMsg;
    PackBaseMsg(sBodyBuffer, iCmd, sBuffer);

    return 0;
}

int Base::PackCheckpointMsg(const CheckpointMsg & oCheckpointMsg, std::string & sBuffer) {
    std::string sBodyBuffer;
    bool is_succeeded = oCheckpointMsg.SerializeToString(&sBodyBuffer);
    if (!is_succeeded) {
        //PLGErr("CheckpointMsg.SerializeToString fail, skip this msg");
        return -1;
    }

    int iCmd = MsgCmd_CheckpointMsg;
    PackBaseMsg(sBodyBuffer, iCmd, sBuffer);

    return 0;
}

void Base::PackBaseMsg(const std::string & sBodyBuffer, const int iCmd, std::string & sBuffer) {
    char sGroupIdx[GROUPIDXLEN] = {0};
    int iGroupIdx = config_->GetMyGroupIdx();
    memcpy(sGroupIdx, &iGroupIdx, sizeof(sGroupIdx));

    Header header;
    header.set_gid(config_->GetGid());
    header.set_rid(0);
    header.set_cmdid(iCmd);
    header.set_version(1);

    std::string sHeaderBuffer;
    bool is_succeeded = header.SerializeToString(&sHeaderBuffer);
    if (!is_succeeded) {
        //PLGErr("Header.SerializeToString fail, skip this msg");
        assert(is_succeeded == true);
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

int Base::UnPackBaseMsg(const std::string & sBuffer, Header & header, size_t & iBodyStartPos, size_t & iBodyLen) {
    uint16_t iHeaderLen = 0;
    memcpy(&iHeaderLen, sBuffer.data() + GROUPIDXLEN, HEADLEN_LEN);

    size_t iHeaderStartPos = GROUPIDXLEN + HEADLEN_LEN;
    iBodyStartPos = iHeaderStartPos + iHeaderLen;

    if (iBodyStartPos > sBuffer.size()) {
        //BP->GetAlgorithmBaseBP()->UnPackHeaderLenTooLong();
        //NLErr("Header headerlen too loog %d", iHeaderLen);
        return -1;
    }

    bool is_succeeded = header.ParseFromArray(sBuffer.data() + iHeaderStartPos, iHeaderLen);
    if (!is_succeeded) {
        //NLErr("Header.ParseFromArray fail, skip this msg");
        return -1;
    }

    //NLDebug("buffer_size %zu header len %d cmdid %d gid %lu rid %lu version %d body_startpos %zu",
    //        sBuffer.size(), iHeaderLen, header.cmdid(), header.gid(), header.rid(), header.version(), iBodyStartPos);

    if (header.version() >= 1) {
        if (iBodyStartPos + CHECKSUM_LEN > sBuffer.size()) {
            //NLErr("no checksum, body start pos %zu buffersize %zu", iBodyStartPos, sBuffer.size());
            return -1;
        }

        iBodyLen = sBuffer.size() - CHECKSUM_LEN - iBodyStartPos;

        uint32_t iBufferChecksum = 0;
        memcpy(&iBufferChecksum, sBuffer.data() + sBuffer.size() - CHECKSUM_LEN, CHECKSUM_LEN);

        uint32_t iNewCalBufferChecksum = crc32(0, (const uint8_t *)sBuffer.data(), sBuffer.size() - CHECKSUM_LEN, NET_CRC32SKIP);
        if (iNewCalBufferChecksum != iBufferChecksum) {
            //BP->GetAlgorithmBaseBP()->UnPackChecksumNotSame();
            return -1;
        }

        /*
        NLDebug("Checksum compare ok, Data.bring.checksum %u, Data.cal.checksum %u",
                iBufferChecksum, iNewCalBufferChecksum) 
        */
    }
    else {
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
    if (ret != 0) {
        return ret;
    }

    return communicate_->SendMessage(config_->GetMyGroupIdx(), iSendtoNodeId, sBuffer, iSendType);
}

int Base::SendMessage(const uint64_t iSendtoNodeId, const PaxosMsg & paxos_msg, const int iSendType) {
    if (is_test_mode_) {
        return 0;
    }

    //BP->GetInstanceBP()->SendMessage();

    if (iSendtoNodeId == config_->GetMyNodeID()) {
        instance_->OnReceivePaxosMsg(paxos_msg);
        return 0;
    }

    std::string sBuffer;
    int ret = PackMsg(paxos_msg, sBuffer);
    if (ret != 0) {
        return ret;
    }

    return communicate_->SendMessage(config_->GetMyGroupIdx(), iSendtoNodeId, sBuffer, iSendType);
}

/* Should remove this logic or rename this part cause all network IO should be
   handled by communicate, which should also be renamed, the author's choice of
   words is terrible :(
   Maybe pass the args in this function to Communicator::BroadcastMessage*/
int Base::BroadcastMessage(const PaxosMsg & paxos_msg, const int iRunType, const int iSendType) {
    if (is_test_mode_) {
        std::cout << "called Base::BroadcastMessage()" << std::endl;
        return 0;
    }

    if (iRunType == BroadcastMessage_Type_RunSelf_First) {
        if (instance_->OnReceivePaxosMsg(paxos_msg) != 0) {
            return -1;
        }
    }

    std::string buffer;
    int ret = PackMsg(paxos_msg, buffer);
    if (ret != 0) {
        return ret;
    }

    ret = communicate_->BroadcastMessage(config_->GetMyGroupIdx(), buffer, iSendType);

    if (iRunType == BroadcastMessage_Type_RunSelf_Final) {
        instance_->OnReceivePaxosMsg(paxos_msg);
    }

    return ret;
}

int Base::BroadcastMessageToFollower(const PaxosMsg & paxos_msg, const int iSendType) {
    std::string sBuffer;
    int ret = PackMsg(paxos_msg, sBuffer);
    if (ret != 0) {
        return ret;
    }

    return communicate_->BroadcastMessageFollower(config_->GetMyGroupIdx(), sBuffer, iSendType);
}

int Base::BroadcastMessageToTempNode(const PaxosMsg & paxos_msg, const int iSendType) {
    std::string sBuffer;
    int ret = PackMsg(paxos_msg, sBuffer);
    if (ret != 0) {
        return ret;
    }

    return communicate_->BroadcastMessageTempNode(config_->GetMyGroupIdx(), sBuffer, iSendType);
}

void Base::SetAsTestMode() {
    is_test_mode_ = true;
}

}

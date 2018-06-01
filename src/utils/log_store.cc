#include "log_store.h"
#include "def.h"
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "crc32.h"
#include "db.h"
#include "paxos_msg.pb.h"
#include "internal_options.h"

namespace paxos {

LogStore::LogStore()
{
    fd_ = -1;
    meta_fd_ = -1;
    file_id_ = -1;
    deleted_max_file_id_ = -1;
    group_idx_ = -1;
    current_file_size_ = -1;
    current_file_offset_ = 0;
}

LogStore::~LogStore()
{
    if (fd_ != -1)
    {
        close(fd_);
    }

    if (meta_fd_ != -1)
    {
        close(meta_fd_);
    }
}

int LogStore::Init(const std::string & sPath, const int iMyGroupIdx, Database * poDatabase)
{
    group_idx_ = iMyGroupIdx;
    path_ = sPath + "/" + "vfile";
    if (access(path_.c_str(), F_OK) == -1)
    {
        if (mkdir(path_.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
        {
            //PLG1Err("Create dir fail, path %s", path_.c_str());
            return -1;
        }
    }

    file_logger_.Init(path_);

    std::string sMetaFilePath = path_ + "/meta";

    meta_fd_ = open(sMetaFilePath.c_str(), O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
    if (meta_fd_ == -1)
    {
        //PLG1Err("open meta file fail, filepath %s", sMetaFilePath.c_str());
        return -1;
    }

    off_t iSeekPos = lseek(meta_fd_, 0, SEEK_SET);
    if (iSeekPos == -1)
    {
        return -1;
    }

    ssize_t iReadLen = read(meta_fd_, &file_id_, sizeof(int));
    if (iReadLen != (ssize_t)sizeof(int))
    {
        if (iReadLen == 0)
        {
            file_id_ = 0;
        }
        else
        {
            //PLG1Err("read meta info fail, readlen %zd", iReadLen);
            return -1;
        }
    }

    uint32_t iMetaChecksum = 0;
    iReadLen = read(meta_fd_, &iMetaChecksum, sizeof(uint32_t));
    if (iReadLen == (ssize_t)sizeof(uint32_t))
    {
        uint32_t iCheckSum = crc32(0, (const uint8_t*)(&file_id_), sizeof(int));
        if (iCheckSum != iMetaChecksum)
        {
            //PLG1Err("meta file checksum %u not same to cal checksum %u, fileid %d",
                    //iMetaChecksum, iCheckSum, file_id_);
            return -2;
        }
    }

    int ret = RebuildIndex(poDatabase, current_file_offset_);
    if (ret != 0)
    {
        //PLG1Err("rebuild index fail, ret %d", ret);
        return -1;
    }

    ret = OpenFile(file_id_, fd_);
    if (ret != 0)
    {
        return ret;
    }

    ret = ExpandFile(fd_, current_file_size_);
    if (ret != 0)
    {
        return ret;
    }

    current_file_offset_ = lseek(fd_, current_file_offset_, SEEK_SET);
    if (current_file_offset_ == -1)
    {
        //PLG1Err("seek to now file offset %d fail", current_file_offset_);
        return -1;
    }

    file_logger_.Log("init write fileid %d now_w_offset %d filesize %d",
            file_id_, current_file_offset_, current_file_size_);

    //PLG1Head("ok, path %s fileid %d meta checksum %u nowfilesize %d nowfilewriteoffset %d",
            //path_.c_str(), file_id_, iMetaChecksum, current_file_size_, current_file_offset_);

    return 0;
}

int LogStore::ExpandFile(int iFd, int & iFileSize)
{
    iFileSize = lseek(iFd, 0, SEEK_END);
    if (iFileSize == -1)
    {
        //PLG1Err("lseek fail, ret %d", iFileSize);
        return -1;
    }

    if (iFileSize == 0)
    {
        //new file
        iFileSize = lseek(iFd, LOG_FILE_MAX_SIZE - 1, SEEK_SET);
        if (iFileSize != LOG_FILE_MAX_SIZE - 1)
        {
            return -1;
        }

        ssize_t iWriteLen = write(iFd, "\0", 1);
        if (iWriteLen != 1)
        {
            //PLG1Err("write 1 bytes fail");
            return -1;
        }

        iFileSize = LOG_FILE_MAX_SIZE;
        int iOffset = lseek(iFd, 0, SEEK_SET);
        current_file_offset_ = 0;
        if (iOffset != 0)
        {
            return -1;
        }
    }

    return 0;
}

int LogStore::IncreaseFileId()
{
    int iFileId = file_id_ + 1;
    uint32_t iCheckSum = crc32(0, (const uint8_t*)(&iFileId), sizeof(int));

    off_t iSeekPos = lseek(meta_fd_, 0, SEEK_SET);
    if (iSeekPos == -1)
    {
        return -1;
    }

    size_t iWriteLen = write(meta_fd_, (char *)&iFileId, sizeof(int));
    if (iWriteLen != sizeof(int))
    {
        //PLG1Err("write meta fileid fail, writelen %zu", iWriteLen);
        return -1;
    }

    iWriteLen = write(meta_fd_, (char *)&iCheckSum, sizeof(uint32_t));
    if (iWriteLen != sizeof(uint32_t))
    {
        //PLG1Err("write meta checksum fail, writelen %zu", iWriteLen);
        return -1;
    }

    int ret = fsync(meta_fd_);
    if (ret != 0)
    {
        return -1;
    }

    file_id_++;

    return 0;
}

int LogStore::OpenFile(const int iFileId, int & iFd)
{
    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", path_.c_str(), iFileId);
    iFd = open(sFilePath, O_CREAT | O_RDWR, S_IWRITE | S_IREAD);
    if (iFd == -1)
    {
        //PLG1Err("open fail fail, filepath %s", sFilePath);
        return -1;
    }

    //PLG1Imp("ok, path %s", sFilePath);
    return 0;
}

int LogStore::DeleteFile(const int iFileId)
{
    if (deleted_max_file_id_ == -1)
    {
        if (iFileId - 2000 > 0)
        {
            deleted_max_file_id_ = iFileId - 2000;
        }
    }

    if (iFileId <= deleted_max_file_id_)
    {
        //PLG1Debug("file already deleted, fileid %d deletedmaxfileid %d", iFileId, deleted_max_file_id_);
        return 0;
    }

    int ret = 0;
    for (int iDeleteFileId = deleted_max_file_id_ + 1; iDeleteFileId <= iFileId; iDeleteFileId++)
    {
        char sFilePath[512] = {0};
        snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", path_.c_str(), iDeleteFileId);

        ret = access(sFilePath, F_OK);
        if (ret == -1)
        {
            //PLG1Debug("file already deleted, filepath %s", sFilePath);
            deleted_max_file_id_ = iDeleteFileId;
            ret = 0;
            continue;
        }

        ret = remove(sFilePath);
        if (ret != 0)
        {
            //PLG1Err("remove fail, filepath %s ret %d", sFilePath, ret);
            break;
        }

        deleted_max_file_id_ = iDeleteFileId;
        file_logger_.Log("delete fileid %d", iDeleteFileId);
    }

    return ret;
}

int LogStore::GetFileFD(const int iNeedWriteSize, int & iFd, int & iFileId, int & iOffset)
{
    if (fd_ == -1)
    {
        //PLG1Err("File aready broken, fileid %d", file_id_);
        return -1;
    }

    iOffset = lseek(fd_, current_file_offset_, SEEK_SET);
    assert(iOffset != -1);

    if (iOffset + iNeedWriteSize > current_file_size_)
    {
        close(fd_);
        fd_ = -1;

        int ret = IncreaseFileId();
        if (ret != 0)
        {
            file_logger_.Log("new file increase fileid fail, now fileid %d", file_id_);
            return ret;
        }

        ret = OpenFile(file_id_, fd_);
        if (ret != 0)
        {
            file_logger_.Log("new file open file fail, now fileid %d", file_id_);
            return ret;
        }

        iOffset = lseek(fd_, 0, SEEK_END);
        if (iOffset != 0)
        {
            assert(iOffset != -1);

            file_logger_.Log("new file but file aready exist, now fileid %d exist filesize %d",
                    file_id_, iOffset);

            //PLG1Err("IncreaseFileId success, but file exist, data wrong, file size %d", iOffset);
            assert(false);
            return -1;
        }

        ret = ExpandFile(fd_, current_file_size_);
        if (ret != 0)
        {
            //PLG1Err("new file expand fail, fileid %d fd %d", file_id_, fd_);

            file_logger_.Log("new file expand file fail, now fileid %d", file_id_);

            close(fd_);
            fd_ = -1;
            return -1;
        }

        file_logger_.Log("new file expand ok, fileid %d filesize %d", file_id_, current_file_size_);
    }

    iFd = fd_;
    iFileId = file_id_;

    return 0;
}

int LogStore::Append(const WriteOptions & oWriteOptions, const uint64_t llInstanceId, const std::string & sBuffer, std::string & sFileId)
{
    time_stat_.Point();
    std::lock_guard<std::mutex> oLock(mutex_);

    int iFd = -1;
    int iFileId = -1;
    int iOffset = -1;

    int iLen = sizeof(uint64_t) + sBuffer.size();
    int iTmpBufferLen = iLen + sizeof(int);

    int ret = GetFileFD(iTmpBufferLen, iFd, iFileId, iOffset);
    if (ret != 0)
    {
        return ret;
    }

    tmp_append_buffer_.Ready(iTmpBufferLen);

    memcpy(tmp_append_buffer_.GetPtr(), &iLen, sizeof(int));
    memcpy(tmp_append_buffer_.GetPtr() + sizeof(int), &llInstanceId, sizeof(uint64_t));
    memcpy(tmp_append_buffer_.GetPtr() + sizeof(int) + sizeof(uint64_t), sBuffer.c_str(), sBuffer.size());

    size_t iWriteLen = write(iFd, tmp_append_buffer_.GetPtr(), iTmpBufferLen);

    if (iWriteLen != (size_t)iTmpBufferLen) {
        return -1;
    }

    if (oWriteOptions.sync)
    {
        int fdatasync_ret = fdatasync(iFd);
        if (fdatasync_ret == -1)
        {
            //PLG1Err("fdatasync fail, writelen %zu errno %d", iWriteLen, errno);
            return -1;
        }
    }

    current_file_offset_ += iWriteLen;

    int iUseTimeMs = time_stat_.Point();
    //BP->GetLogStorageBP()->AppendDataOK(iWriteLen, iUseTimeMs);

    uint32_t iCheckSum = crc32(0, (const uint8_t*)(tmp_append_buffer_.GetPtr() + sizeof(int)), iTmpBufferLen - sizeof(int), CRC32SKIP);

    GenFileId(iFileId, iOffset, iCheckSum, sFileId);

    //PLG1Imp("ok, offset %d fileid %d checksum %u instanceid %lu buffer size %zu usetime %dms sync %d",
            //iOffset, iFileId, iCheckSum, llInstanceId, sBuffer.size(), iUseTimeMs, (int)oWriteOptions.bSync);

    return 0;
}

int LogStore::Read(const std::string & sFileId, uint64_t & llInstanceId, std::string & sBuffer) {
    int iFileId = -1;
    int iOffset = -1;
    uint32_t iCheckSum = 0;
    ParseFileId(sFileId, iFileId, iOffset, iCheckSum);

    int iFd = -1;
    int ret = OpenFile(iFileId, iFd);
    if (ret != 0) {
        return ret;
    }

    off_t iSeekPos = lseek(iFd, iOffset, SEEK_SET);
    if (iSeekPos == -1) {
        return -1;
    }

    int iLen = 0;
    ssize_t iReadLen = read(iFd, (char *)&iLen, sizeof(int));
    if (iReadLen != (ssize_t)sizeof(int)) {
        close(iFd);
        //PLG1Err("readlen %zd not qual to %zu", iReadLen, sizeof(int));
        return -1;
    }

    std::lock_guard<std::mutex> oLock(read_mutex_);

    tmp_buffer_.Ready(iLen);
    iReadLen = read(iFd, tmp_buffer_.GetPtr(), iLen);
    if (iReadLen != iLen) {
        close(iFd);
        return -1;
    }

    close(iFd);

    uint32_t iFileCheckSum = crc32(0, (const uint8_t *)tmp_buffer_.GetPtr(), iLen, CRC32SKIP);

    if (iFileCheckSum != iCheckSum)
    {
        //BP->GetLogStorageBP()->GetFileChecksumNotEquel();
        //PLG1Err("checksum not equal, filechecksum %u checksum %u", iFileCheckSum, iCheckSum);
        return -2;
    }

    memcpy(&llInstanceId, tmp_buffer_.GetPtr(), sizeof(uint64_t));
    sBuffer = std::string(tmp_buffer_.GetPtr() + sizeof(uint64_t), iLen - sizeof(uint64_t));

    return 0;
}

int LogStore::Del(const std::string & sFileId, const uint64_t llInstanceId) {
    int iFileId = -1;
    int iOffset = -1;
    uint32_t iCheckSum = 0;
    ParseFileId(sFileId, iFileId, iOffset, iCheckSum);

    if (iFileId > file_id_) {
        //PLG1Err("del fileid %d large than useing fileid %d", iFileId, file_id_);
        return -2;
    }

    if (iFileId > 0) {
        return DeleteFile(iFileId - 1);
    }

    return 0;
}

int LogStore::ForceDel(const std::string & sFileId, const uint64_t llInstanceId)
{
    int iFileId = -1;
    int iOffset = -1;
    uint32_t iCheckSum = 0;
    ParseFileId(sFileId, iFileId, iOffset, iCheckSum);

    if (iFileId != file_id_)
    {
        //PLG1Err("del fileid %d not equal to fileid %d", iFileId, file_id_);
        return -2;
    }

    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", path_.c_str(), iFileId);

    printf("fileid %d offset %d\n", iFileId, iOffset);

    if (truncate(sFilePath, iOffset) != 0)
    {
        return -1;
    }

    return 0;
}


void LogStore::GenFileId(const int iFileId, const int iOffset, const uint32_t iCheckSum, std::string & sFileId)
{
    char sTmp[sizeof(int) + sizeof(int) + sizeof(uint32_t)] = {0};
    memcpy(sTmp, (char *)&iFileId, sizeof(int));
    memcpy(sTmp + sizeof(int), (char *)&iOffset, sizeof(int));
    memcpy(sTmp + sizeof(int) + sizeof(int), (char *)&iCheckSum, sizeof(uint32_t));

    sFileId = std::string(sTmp, sizeof(int) + sizeof(int) + sizeof(uint32_t));
}

void LogStore::ParseFileId(const std::string & sFileId, int & iFileId, int & iOffset, uint32_t & iCheckSum)
{
    memcpy(&iFileId, (void *)sFileId.c_str(), sizeof(int));
    memcpy(&iOffset, (void *)(sFileId.c_str() + sizeof(int)), sizeof(int));
    memcpy(&iCheckSum, (void *)(sFileId.c_str() + sizeof(int) + sizeof(int)), sizeof(uint32_t));

    //PLG1Debug("fileid %d offset %d checksum %u", iFileId, iOffset, iCheckSum);
}

const bool LogStore::IsValidFileId(const std::string & sFileId)
{
    if (sFileId.size() != FILEId_LEN)
    {
        return false;
    }

    return true;
}

int LogStore::RebuildIndex(Database * poDatabase, int & iNowFileWriteOffset)
{
    std::string sLastFileId;

    uint64_t llNowInstanceId = 0;
    int ret = poDatabase->GetMaxInstanceIDFileID(sLastFileId, llNowInstanceId);
    if (ret != 0)
    {
        return ret;
    }

    int iFileId = 0;
    int iOffset = 0;
    uint32_t iCheckSum = 0;

    if (sLastFileId.size() > 0)
    {
        ParseFileId(sLastFileId, iFileId, iOffset, iCheckSum);
    }

    if (iFileId > file_id_)
    {
        //PLG1Err("LevelDB last fileid %d larger than meta now fileid %d, file error",
                //iFileId, file_id_);
        return -2;
    }

    //PLG1Head("START fileid %d offset %d checksum %u", iFileId, iOffset, iCheckSum);

    for (int iNowFileId = iFileId; ;iNowFileId++)
    {
        ret = RebuildIndexForOneFile(iNowFileId, iOffset, poDatabase, iNowFileWriteOffset, llNowInstanceId);
        if (ret != 0 && ret != 1)
        {
            break;
        }
        else if (ret == 1)
        {
            if (iNowFileId != 0 && iNowFileId != file_id_ + 1)
            {
                //PLG1Err("meta file wrong, nowfileid %d meta.nowfileid %d", iNowFileId, file_id_);
                return -1;
            }

            ret = 0;
            //PLG1Imp("END rebuild ok, nowfileid %d", iNowFileId);
            break;
        }

        iOffset = 0;
    }

    return ret;
}

int LogStore::RebuildIndexForOneFile(const int iFileId, const int iOffset,
        Database * poDatabase, int & iNowFileWriteOffset, uint64_t & llNowInstanceId) {
    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/%d.f", path_.c_str(), iFileId);

    int ret = access(sFilePath, F_OK);
    if (ret == -1) {
        //PLG1Debug("file not exist, filepath %s", sFilePath);
        return 1;
    }

    int iFd = -1;
    ret = OpenFile(iFileId, iFd);
    if (ret != 0)
    {
        return ret;
    }

    int iFileLen = lseek(iFd, 0, SEEK_END);
    if (iFileLen == -1)
    {
        close(iFd);
        return -1;
    }

    off_t iSeekPos = lseek(iFd, iOffset, SEEK_SET);
    if (iSeekPos == -1)
    {
        close(iFd);
        return -1;
    }

    int iNowOffset = iOffset;
    bool bNeedTruncate = false;

    while (true)
    {
        int iLen = 0;
        ssize_t iReadLen = read(iFd, (char *)&iLen, sizeof(int));
        if (iReadLen == 0)
        {
            //PLG1Head("File End, fileid %d offset %d", iFileId, iNowOffset);
            iNowFileWriteOffset = iNowOffset;
            break;
        }

        if (iReadLen != (ssize_t)sizeof(int))
        {
            bNeedTruncate = true;
            //PLG1Err("readlen %zd not qual to %zu, need truncate", iReadLen, sizeof(int));
            break;
        }

        if (iLen == 0)
        {
            //PLG1Head("File Data End, fileid %d offset %d", iFileId, iNowOffset);
            iNowFileWriteOffset = iNowOffset;
            break;
        }

        if (iLen > iFileLen || iLen < (int)sizeof(uint64_t))
        {
            //PLG1Err("File data len wrong, data len %d filelen %d",
                    //iLen, iFileLen);
            ret = -1;
            break;
        }

        tmp_buffer_.Ready(iLen);
        iReadLen = read(iFd, tmp_buffer_.GetPtr(), iLen);
        if (iReadLen != iLen)
        {
            bNeedTruncate = true;
            //PLG1Err("readlen %zd not qual to %zu, need truncate", iReadLen, iLen);
            break;
        }


        uint64_t llInstanceId = 0;
        memcpy(&llInstanceId, tmp_buffer_.GetPtr(), sizeof(uint64_t));

        //InstanceId must be ascending order.
        if (llInstanceId < llNowInstanceId)
        {
            //PLG1Err("File data wrong, read instanceid %lu smaller than now instanceid %lu",
                    //llInstanceId, llNowInstanceId);
            ret = -1;
            break;
        }
        llNowInstanceId = llInstanceId;

        AcceptorStateData oState;
        bool bBufferValid = oState.ParseFromArray(tmp_buffer_.GetPtr() + sizeof(uint64_t), iLen - sizeof(uint64_t));
        if (!bBufferValid)
        {
            current_file_offset_ = iNowOffset;
            //PLG1Err("This instance's buffer wrong, can't parse to acceptState, instanceid %lu bufferlen %d nowoffset %d",
                    //llInstanceId, iLen - sizeof(uint64_t), iNowOffset);
            bNeedTruncate = true;
            break;
        }

        uint32_t iFileCheckSum = crc32(0, (const uint8_t *)tmp_buffer_.GetPtr(), iLen, CRC32SKIP);

        std::string sFileId;
        GenFileId(iFileId, iNowOffset, iFileCheckSum, sFileId);

        ret = poDatabase->RebuildOneIndex(llInstanceId, sFileId);
        if (ret != 0)
        {
            break;
        }

        //PLG1Imp("rebuild one index ok, fileid %d offset %d instanceid %lu checksum %u buffer size %zu",
                //iFileId, iNowOffset, llInstanceId, iFileCheckSum, iLen - sizeof(uint64_t));

        iNowOffset += sizeof(int) + iLen;
    }

    close(iFd);

    if (bNeedTruncate)
    {
        file_logger_.Log("truncate fileid %d offset %d filesize %d",
                iFileId, iNowOffset, iFileLen);
        if (truncate(sFilePath, iNowOffset) != 0)
        {
            //PLG1Err("truncate fail, file path %s truncate to length %d errno %d",
                    //sFilePath, iNowOffset, errno);
            return -1;
        }
    }

    return ret;
}

//////////////////////////////////////////////////////////

LogStoreLogger::LogStoreLogger()
    : log_fd_(-1)
{
}

LogStoreLogger::~LogStoreLogger()
{
    if (log_fd_ != -1)
    {
        close(log_fd_);
    }
}

void LogStoreLogger::Init(const std::string & sPath)
{
    char sFilePath[512] = {0};
    snprintf(sFilePath, sizeof(sFilePath), "%s/LOG", sPath.c_str());
    log_fd_ = open(sFilePath, O_CREAT | O_RDWR | O_APPEND, S_IWRITE | S_IREAD);
}

void LogStoreLogger::Log(const char * pcFormat, ...) {
    if (log_fd_ == -1) {
        return;
    }

    uint64_t llNowTime = Time::GetTimestampMS();
    time_t tNowTimeSeconds = (time_t)(llNowTime / 1000);
    //tm * local_time = localtime(&tNowTimeSeconds);
    tm * local_time = nullptr;
    local_time = localtime_r(&tNowTimeSeconds, local_time);
    char sTimePrefix[64] = {0};
    strftime(sTimePrefix, sizeof(sTimePrefix), "%Y-%m-%d %H:%M:%S", local_time);

    char sPrefix[128] = {0};
    snprintf(sPrefix, sizeof(sPrefix), "%s:%d ", sTimePrefix, (int)(llNowTime % 1000));
    std::string sNewFormat = std::string(sPrefix) + pcFormat + "\n";

    char sBuf[1024] = {0};
    va_list args;
    va_start(args, pcFormat);
    vsnprintf(sBuf, sizeof(sBuf), sNewFormat.c_str(), args);
    va_end(args);

    int iLen = strnlen(sBuf, sizeof(sBuf));
    ssize_t iWriteLen = write(log_fd_, sBuf, iLen);
    if (iWriteLen != iLen)
    {
        //PLErr("fail, len %d writelen %d", iLen, iWriteLen);
    }
}   
}

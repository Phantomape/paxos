#pragma once

#include "bytes_buffer.h"
#include "log_storage.h"
#include "util.h"
#include <string>
#include <mutex>

namespace paxos {

class Database;

#define FILEId_LEN (sizeof(int) + sizeof(int) + sizeof(uint32_t))

class LogStoreLogger {
public:
    LogStoreLogger();

    ~LogStoreLogger();

    void Init(const std::string & path);

    void Log(const char * pcFormat, ...);

private:
    int log_fd_;
};

class LogStore {
public:
    LogStore();

    ~LogStore();

    int Init(const std::string & path, const int iMyGroupIdx, Database * database);

    int Append(const WriteOptions & write_options, const uint64_t instance_id, const std::string & sBuffer, std::string & file_id);

    int Read(const std::string & file_id, uint64_t & instance_id, std::string & sBuffer);

    int Del(const std::string & file_id, const uint64_t instance_id);

    int ForceDel(const std::string & file_id, const uint64_t instance_id);

    const bool IsValidFileId(const std::string & file_id);
    
    int RebuildIndex(Database * database, int & current_file_write_offset);

    int RebuildIndexForOneFile(const int file_id, const int offset, 
            Database * database, int & current_file_write_offset, uint64_t & llNowInstanceId);

private:
    void GenFileId(const int int_file_id, const int offset, const uint32_t iCheckSum, std::string & file_id);

    void ParseFileId(const std::string & file_id, int & int_file_id, int & offset, uint32_t & iCheckSum);

    int IncreaseFileId();

    int OpenFile(const int file_id, int & fd);

    int DeleteFile(const int file_id);

    int GetFileFD(const int need_write_size, int & fd, int & file_id, int & offset);

    int ExpandFile(int fd, int & iFileSize);
    
private:
    int fd_;
    int meta_fd_;
    int file_id_;
    std::string path_;
    BytesBuffer tmp_buffer_;
    BytesBuffer tmp_append_buffer_;

    std::mutex mutex_;
    std::mutex read_mutex_;

    int deleted_max_file_id_;
    int group_idx_;

    int current_file_size_;
    int current_file_offset_;

private:
    TimeStat time_stat_;
    LogStoreLogger file_logger_;
};

}

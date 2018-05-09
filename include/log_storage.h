#pragma once

#include <string>
#include <typeinfo>
#include <inttypes.h>

namespace paxos {

//Paxoslib need to storage many datas, if you want to storage datas yourself,
//you must implememt all function in class LogStorage, and make sure that observe 
//the writeoptions.

class WriteOptions {
public:
    WriteOptions() : sync(true) { }
    bool sync;
};

class LogStorage {
public:
    virtual ~LogStorage() {}

    virtual const std::string GetLogStorageDirPath(const int group_idx) = 0;

    virtual int Get(const int group_idx, const uint64_t instance_id, std::string & val) = 0;

    virtual int Put(const WriteOptions & write_options, const int group_idx, const uint64_t instance_id, const std::string & val) = 0;

    virtual int Del(const WriteOptions & write_options, int group_idx, const uint64_t instance_id) = 0;

    virtual int GetMaxInstanceID(const int group_idx, uint64_t & instance_id) = 0;

    virtual int SetMinChosenInstanceID(const WriteOptions & write_options, const int group_idx, const uint64_t min_instance_id) = 0;

    virtual int GetMinChosenInstanceID(const int group_idx, uint64_t & min_instance_id) = 0;

    virtual int ClearAllLog(const int group_idx) = 0;

    virtual int SetSystemVariables(const WriteOptions & write_options, const int group_idx, const std::string & buffer) = 0;

    virtual int GetSystemVariables(const int group_idx, std::string & buffer) = 0;

    virtual int SetMasterVariables(const WriteOptions & write_options, const int group_idx, const std::string & buffer) = 0;

    virtual int GetMasterVariables(const int group_idx, std::string & buffer) = 0;
};

}

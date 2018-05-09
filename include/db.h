/*
Tencent is pleased to support the open source community by making 
PhxPaxos available.
Copyright (C) 2016 THL A29 Limited, a Tencent company. 
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may 
not use this file except in compliance with the License. You may 
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" basis, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
implied. See the License for the specific language governing 
permissions and limitations under the License.

See the AUTHORS file for names of contributors. 
*/

#pragma once

#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include <vector>
#include <string>
#include <map>

namespace paxos {

class PaxosComparator : public leveldb::Comparator {
public:
    int Compare(const leveldb::Slice & a, const leveldb::Slice & b) const;
    
    static int PCompare(const leveldb::Slice & a, const leveldb::Slice & b);

    const char * Name() const {return "PaxosComparator";}

    void FindShortestSeparator(std::string *, const leveldb::Slice &) const {}

    void FindShortSuccessor(std::string *) const {}
};

#define MINCHOSEN_KEY ((uint64_t)-1)
#define SYSTEMVARIABLES_KEY ((uint64_t)-2)
#define MASTERVARIABLES_KEY ((uint64_t)-3)

class Database {
public:
    Database();

    ~Database();

    int Init(const std::string & db_path, const int group_idx);

    const std::string GetDbPath();

    int ClearAllLog();

    int Get(const uint64_t instance_id, std::string & val);

    int Put(const WriteOptions & write_options, const uint64_t instance_id, const std::string & val);

    int Del(const WriteOptions & write_options, const uint64_t instance_id);

    int ForceDel(const WriteOptions & write_options, const uint64_t instance_id);

    int GetMaxInstanceID(uint64_t & instance_id);

    int SetMinChosenInstanceID(const WriteOptions & write_options, const uint64_t llMinInstanceID);

    int GetMinChosenInstanceID(uint64_t & llMinInstanceID);

    int SetSystemVariables(const WriteOptions & write_options, const std::string & buffer);

    int GetSystemVariables(std::string & buffer);

    int SetMasterVariables(const WriteOptions & write_options, const std::string & buffer);

    int GetMasterVariables(std::string & buffer);
    
public:
    int GetMaxInstanceIDFileID(std::string & file_id, uint64_t & instance_id);

    int RebuildOneIndex(const uint64_t instance_id, const std::string & file_id);
    
private:
    int ValueToFileID(const WriteOptions & write_options, const uint64_t instance_id, const std::string & val, std::string & file_id);

    int FileIDToValue(const std::string & file_id, uint64_t & instance_id, std::string & val);

    int GetFromLevelDb(const uint64_t instance_id, std::string & val);

    int PutToLevelDb(const bool bSync, const uint64_t instance_id, const std::string & val);
        
private:
    std::string GenKey(const uint64_t instance_id);

    const uint64_t GetInstanceIDFromKey(const std::string & sKey);

public:
//private:
    leveldb::Db * m_poLevelDb;
    PaxosComparator m_oPaxosCmp;
    bool m_bHasInit;
    
    LogStore * m_poValueStore;
    std::string m_db_path;

    int m_group_idx;

private:
    TimeStat m_oTimeStat;
};

//////////////////////////////////////////

class MultiDatabase : public LogStorage
{
public:
    MultiDatabase();
    ~MultiDatabase();

    int Init(const std::string & db_path, const int iGroupCount);

    const std::string GetLogStorageDirPath(const int group_idx);

    int Get(const int group_idx, const uint64_t instance_id, std::string & val);

    int Put(const WriteOptions & write_options, const int group_idx, const uint64_t instance_id, const std::string & val);

    int Del(const WriteOptions & write_options, const int group_idx, const uint64_t instance_id);

    int ForceDel(const WriteOptions & write_options, const int group_idx, const uint64_t instance_id);

    int GetMaxInstanceID(const int group_idx, uint64_t & instance_id);

    int SetMinChosenInstanceID(const WriteOptions & write_options, const int group_idx, const uint64_t llMinInstanceID);

    int GetMinChosenInstanceID(const int group_idx, uint64_t & llMinInstanceID);

    int ClearAllLog(const int group_idx);

    int SetSystemVariables(const WriteOptions & write_options, const int group_idx, const std::string & buffer);

    int GetSystemVariables(const int group_idx, std::string & buffer);
    
    int SetMasterVariables(const WriteOptions & write_options, const int group_idx, const std::string & buffer);

    int GetMasterVariables(const int group_idx, std::string & buffer);

private:
    std::vector<Database *> m_vecDbList;
};

}
    


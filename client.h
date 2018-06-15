#pragma once

#include "leveldb/db.h"
#include <string>
#include <mutex>

namespace paxoskv {

enum KVClientRet {
    KVCLIENT_OK = 0,
    KVCLIENT_SYS_FAIL = -1,
    KVCLIENT_KEY_NOTEXIST = 1,
    KVCLIENT_KEY_VERSION_CONFLICT = -11,
};

#define KV_CHECKPOINT_KEY ((uint64_t)-1)

class KVClient {
public:
    KVClient();
    
    ~KVClient();

    bool Init(const std::string & sDBPath);

    static KVClient * Instance();

    KVClientRet Get(const std::string & sKey, std::string & sValue, uint64_t & llVersion);

    KVClientRet Set(const std::string & sKey, const std::string & sValue, const uint64_t llVersion);

    KVClientRet Del(const std::string & sKey, const uint64_t llVersion);

    KVClientRet GetCheckpointInstanceID(uint64_t & llCheckpointInstanceID);

    KVClientRet SetCheckpointInstanceID(const uint64_t llCheckpointInstanceID);

private:
    leveldb::DB * m_poLevelDB;
    bool m_bHasInit;
    std::mutex m_oMutex;
};

}

#include "client.h"
#include "rpc.pb.h"
#include "log.h"

namespace paxoskv {

KVClient::KVClient() {
    m_bHasInit = false;
    m_poLevelDB = nullptr;
}

KVClient::~KVClient() {
}

bool KVClient::Init(const std::string & sDBPath) {
    if (m_bHasInit) {
        return true;
    }
    
    leveldb::Options oOptions;
    oOptions.create_if_missing = true;
    leveldb::Status oStatus = leveldb::DB::Open(oOptions, sDBPath, &m_poLevelDB);

    if (!oStatus.ok()) {
        return false;
    }

    m_bHasInit = true;

    return true;
}

KVClient * KVClient::Instance() {
    static KVClient oKVClient;
    return &oKVClient;
}

KVClientRet KVClient::Get(const std::string & sKey, std::string & sValue, uint64_t & llVersion) {
    if (!m_bHasInit) {
        return KVCLIENT_SYS_FAIL;
    }

    std::string sBuffer;
    leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sBuffer);
    if (!oStatus.ok()) {
        if (oStatus.IsNotFound()) {
            llVersion = 0;
            return KVCLIENT_KEY_NOTEXIST;
        }
        
        return KVCLIENT_SYS_FAIL;
    }

    KVData data;
    bool bSucc = data.ParseFromArray(sBuffer.data(), sBuffer.size());
    if (!bSucc) {
        return KVCLIENT_SYS_FAIL;
    }

    llVersion = data.version();

    if (data.isdeleted()) {
        return KVCLIENT_KEY_NOTEXIST;
    }

    sValue = data.value();

    return KVCLIENT_OK;
}

KVClientRet KVClient::Set(const std::string & sKey, const std::string & sValue, const uint64_t llVersion) {
    if (!m_bHasInit) {
        return KVCLIENT_SYS_FAIL;
    }

    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    uint64_t llServerVersion = 0;
    std::string sServerValue;
    KVClientRet ret = Get(sKey, sServerValue, llServerVersion);
    if (ret != KVCLIENT_OK && ret != KVCLIENT_KEY_NOTEXIST) {
        return KVCLIENT_SYS_FAIL;
    }

    if (llServerVersion != llVersion) {
        return KVCLIENT_KEY_VERSION_CONFLICT;
    }

    llServerVersion++;
    KVData data;
    data.set_value(sValue);
    data.set_version(llServerVersion);
    data.set_isdeleted(false);

    std::string sBuffer;
    bool bSucc = data.SerializeToString(&sBuffer);
    if (!bSucc) {
        return KVCLIENT_SYS_FAIL;
    }
    
    leveldb::Status oStatus = m_poLevelDB->Put(leveldb::WriteOptions(), sKey, sBuffer);
    if (!oStatus.ok()) {
        return KVCLIENT_SYS_FAIL;
    }

    return KVCLIENT_OK;
}

KVClientRet KVClient::Del(const std::string & sKey, const uint64_t llVersion) {
    if (!m_bHasInit) {
        return KVCLIENT_SYS_FAIL;
    }

    std::lock_guard<std::mutex> oLockGuard(m_oMutex);

    uint64_t llServerVersion = 0;
    std::string sServerValue;
    KVClientRet ret = Get(sKey, sServerValue, llServerVersion);
    if (ret != KVCLIENT_OK && ret != KVCLIENT_KEY_NOTEXIST) {
        return KVCLIENT_SYS_FAIL;
    }

    if (llServerVersion != llVersion) {
        return KVCLIENT_KEY_VERSION_CONFLICT;
    }

    llServerVersion++;
    KVData data;
    data.set_value(sServerValue);
    data.set_version(llServerVersion);
    data.set_isdeleted(true);

    std::string sBuffer;
    bool bSucc = data.SerializeToString(&sBuffer);
    if (!bSucc) {
        return KVCLIENT_SYS_FAIL;
    }

    leveldb::Status oStatus = m_poLevelDB->Put(leveldb::WriteOptions(), sKey, sBuffer);
    if (!oStatus.ok()) {
        return KVCLIENT_SYS_FAIL;
    }

    return KVCLIENT_OK;
}

KVClientRet KVClient::GetCheckpointInstanceID(uint64_t & llCheckpointInstanceID) {
    if (!m_bHasInit) {
        return KVCLIENT_SYS_FAIL;
    }

    std::string sKey;
    static uint64_t llCheckpointInstanceIDKey = KV_CHECKPOINT_KEY;
    sKey.append((char *)&llCheckpointInstanceIDKey, sizeof(uint64_t));

    std::string sBuffer;
    leveldb::Status oStatus = m_poLevelDB->Get(leveldb::ReadOptions(), sKey, &sBuffer);
    if (!oStatus.ok()) {
        if (oStatus.IsNotFound()) {
            return KVCLIENT_KEY_NOTEXIST;
        }

        return KVCLIENT_SYS_FAIL;
    }

    memcpy(&llCheckpointInstanceID, sBuffer.data(), sizeof(uint64_t));

    return KVCLIENT_OK;
}

KVClientRet KVClient::SetCheckpointInstanceID(const uint64_t llCheckpointInstanceID) {
    if (!m_bHasInit) {
        return KVCLIENT_SYS_FAIL;
    }

    std::string sKey;
    static uint64_t llCheckpointInstanceIDKey = KV_CHECKPOINT_KEY;
    sKey.append((char *)&llCheckpointInstanceIDKey, sizeof(uint64_t));

    std::string sBuffer;
    sBuffer.append((char *)&llCheckpointInstanceID, sizeof(uint64_t));

    leveldb::WriteOptions oWriteOptions;
    //must fync
    oWriteOptions.sync = true;

    leveldb::Status oStatus = m_poLevelDB->Put(oWriteOptions, sKey, sBuffer);
    if (!oStatus.ok()) {
        return KVCLIENT_SYS_FAIL;
    }

    return KVCLIENT_OK;
}

}

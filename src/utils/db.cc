#include "db.h"
#include <unistd.h>

namespace paxos {

int PaxosComparator::Compare(const leveldb::Slice & a, const leveldb::Slice & b) const {
    return PCompare(a, b);
}

int PaxosComparator::PCompare(const leveldb::Slice & a, const leveldb::Slice & b) {
    if (a.size() != sizeof(uint64_t)) {
        //NLErr("assert a.size %zu b.size %zu", a.size(), b.size());
        assert(a.size() == sizeof(uint64_t));
    }

    if (b.size() != sizeof(uint64_t)) {
        //NLErr("assert a.size %zu b.size %zu", a.size(), b.size());
        assert(b.size() == sizeof(uint64_t));
    }

    uint64_t lla = 0;
    uint64_t llb = 0;

    memcpy(&lla, a.data(), sizeof(uint64_t));
    memcpy(&llb, b.data(), sizeof(uint64_t));

    if (lla == llb) {
        return 0;
    }

    return lla < llb ? -1 : 1;
}

Database::Database() : leveldb_(nullptr), value_store_(nullptr) {
    has_init_ = false;
    group_idx_ = -1;
}

Database::~Database() {
    delete value_store_;
    delete leveldb_;

    ////PLG1Head("LevelDB Deleted. Path %s", db_path_.c_str());
}

int Database::ClearAllLog() {
    std::string sSystemVariablesBuffer;
    int ret = GetSystemVariables(sSystemVariablesBuffer);
    if (ret != 0 && ret != 1) {
        //PLG1Err("GetSystemVariables fail, ret %d", ret);
        return ret;
    }

    std::string sMasterVariablesBuffer;
    ret = GetMasterVariables(sMasterVariablesBuffer);
    if (ret != 0 && ret != 1) {
        //PLG1Err("GetMasterVariables fail, ret %d", ret);
        return ret;
    }

    has_init_ = false;

    delete leveldb_;
    leveldb_ = nullptr;

    delete value_store_;
    value_store_ = nullptr;

    std::string sBakPath = db_path_ + ".bak";

    ret = FileUtils::DeleteDir(sBakPath);
    if (ret != 0) {
        //PLG1Err("Delete bak dir fail, dir %s", sBakPath.c_str());
        return -1;
    }

    ret = rename(db_path_.c_str(), sBakPath.c_str());
    assert(ret == 0);

    ret = Init(db_path_, group_idx_);
    if (ret != 0) {
        //PLG1Err("Init again fail, ret %d", ret);
        return ret;
    }

    WriteOptions oWriteOptions;
    oWriteOptions.sync = true;
    if (sSystemVariablesBuffer.size() > 0) {
        ret = SetSystemVariables(oWriteOptions, sSystemVariablesBuffer);
        if (ret != 0) {
            //PLG1Err("SetSystemVariables fail, ret %d", ret);
            return ret;
        }
    }

    if (sMasterVariablesBuffer.size() > 0) {
        ret = SetMasterVariables(oWriteOptions, sMasterVariablesBuffer);
        if (ret != 0) {
            //PLG1Err("SetMasterVariables fail, ret %d", ret);
            return ret;
        }
    }

    return 0;
}

int Database::Init(const std::string & sDBPath, const int iMyGroupIdx) {
    if (has_init_) {
        return 0;
    }

    group_idx_ = iMyGroupIdx;

    db_path_ = sDBPath;

    leveldb::Options oOptions;
    oOptions.create_if_missing = true;
    oOptions.comparator = &paxos_cmp_;
    //every group have different buffer size to avoid all group compact at the same time.
    oOptions.write_buffer_size = 1024 * 1024 + iMyGroupIdx * 10 * 1024;

    leveldb::Status oStatus = leveldb::DB::Open(oOptions, sDBPath, &leveldb_);

    if (!oStatus.ok()) {
        //PLG1Err("Open leveldb fail, db_path %s", sDBPath.c_str());
        return -1;
    }

    value_store_ = new LogStore();
    assert(value_store_ != nullptr);

    int ret = value_store_->Init(sDBPath, iMyGroupIdx, (Database *)this);
    if (ret != 0) {
        //PLG1Err("value store init fail, ret %d", ret);
        return -1;
    }

    has_init_ = true;

    //PLG1Imp("OK, db_path %s", sDBPath.c_str());

    return 0;
}

const std::string Database::GetDbPath() {
    return db_path_;
}

int Database::GetMaxInstanceIDFileID(std::string & sFileID, uint64_t & llInstanceID) {
    uint64_t llMaxInstanceID = 0;
    int ret = GetMaxInstanceID(llMaxInstanceID);
    if (ret != 0 && ret != 1) {
        return ret;
    }

    if (ret == 1) {
        sFileID = "";
        return 0;
    }

    std::string sKey = GenKey(llMaxInstanceID);

    leveldb::Status oStatus = leveldb_->Get(leveldb::ReadOptions(), sKey, &sFileID);
    if (!oStatus.ok()) {
        if (oStatus.IsNotFound()) {
            //BP->GetLogStorageBP()->LevelDBGetNotExist();
            ////PLG1Err("LevelDB.Get not found %s", sKey.c_str());
            return 1;
        }

        //BP->GetLogStorageBP()->LevelDBGetFail();
        //PLG1Err("LevelDB.Get fail");
        return -1;
    }

    llInstanceID = llMaxInstanceID;

    return 0;
}

int Database::RebuildOneIndex(const uint64_t llInstanceID, const std::string & sFileID) {
    std::string sKey = GenKey(llInstanceID);

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = false;

    leveldb::Status oStatus = leveldb_->Put(oLevelDBWriteOptions, sKey, sFileID);
    if (!oStatus.ok()) {
        //BP->GetLogStorageBP()->LevelDBPutFail();
        //PLG1Err("LevelDB.Put fail, instanceid %lu valuelen %zu", llInstanceID, sFileID.size());
        return -1;
    }

    return 0;
}

int Database::GetFromLevelDb(const uint64_t llInstanceID, std::string & sValue) {
    std::string sKey = GenKey(llInstanceID);

    leveldb::Status oStatus = leveldb_->Get(leveldb::ReadOptions(), sKey, &sValue);
    if (!oStatus.ok()) {
        if (oStatus.IsNotFound()) {
            //BP->GetLogStorageBP()->LevelDBGetNotExist();
            //PLG1Debug("LevelDB.Get not found, instanceid %lu", llInstanceID);
            return 1;
        }

        //BP->GetLogStorageBP()->LevelDBGetFail();
        //PLG1Err("LevelDB.Get fail, instanceid %lu", llInstanceID);
        return -1;
    }

    return 0;
}

int Database::Get(const uint64_t llInstanceID, std::string & sValue) {
    if (!has_init_) {
        //PLG1Err("no init yet");
        return -1;
    }

    std::string sFileID;
    int ret = GetFromLevelDb(llInstanceID, sFileID);
    if (ret != 0) {
        return ret;
    }

    uint64_t llFileInstanceID = 0;
    ret = FileIDToValue(sFileID, llFileInstanceID, sValue);
    if (ret != 0) {
        //BP->GetLogStorageBP()->FileIDToValueFail();
        return ret;
    }

    if (llFileInstanceID != llInstanceID) {
        //PLG1Err("file instanceid %lu not equal to key.instanceid %lu", llFileInstanceID, llInstanceID);
        return -2;
    }

    return 0;
}

int Database::ValueToFileID(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sValue, std::string & sFileID) {
    int ret = value_store_->Append(oWriteOptions, llInstanceID, sValue, sFileID);
    if (ret != 0) {
        //BP->GetLogStorageBP()->ValueToFileIDFail();
        //PLG1Err("fail, ret %d", ret);
        return ret;
    }

    return 0;
}

int Database::FileIDToValue(const std::string & sFileID, uint64_t & llInstanceID, std::string & sValue) {
    int ret = value_store_->Read(sFileID, llInstanceID, sValue);
    if (ret != 0) {
        //PLG1Err("fail, ret %d", ret);
        return ret;
    }

    return 0;
}

int Database::PutToLevelDb(const bool sync, const uint64_t llInstanceID, const std::string & sValue) {
    std::string sKey = GenKey(llInstanceID);

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = sync;

    time_stat_.Point();

    leveldb::Status oStatus = leveldb_->Put(oLevelDBWriteOptions, sKey, sValue);
    if (!oStatus.ok()) {
        //BP->GetLogStorageBP()->LevelDBPutFail();
        //PLG1Err("LevelDB.Put fail, instanceid %lu valuelen %zu", llInstanceID, sValue.size());
        return -1;
    }

    //BP->GetLogStorageBP()->LevelDBPutOK(m_oTimeStat.Point());

    return 0;
}

int Database::Put(const WriteOptions & oWriteOptions, const uint64_t llInstanceID, const std::string & sValue) {
    if (!has_init_) {
        //PLG1Err("no init yet");
        return -1;
    }

    std::string sFileID;
    int ret = ValueToFileID(oWriteOptions, llInstanceID, sValue, sFileID);
    if (ret != 0) {
        return ret;
    }

    ret = PutToLevelDb(false, llInstanceID, sFileID);

    return ret;
}

int Database::ForceDel(const WriteOptions & oWriteOptions, const uint64_t llInstanceID) {
    if (!has_init_) {
        //PLG1Err("no init yet");
        return -1;
    }

    std::string sKey = GenKey(llInstanceID);
    std::string sFileID;

    leveldb::Status oStatus = leveldb_->Get(leveldb::ReadOptions(), sKey, &sFileID);
    if (!oStatus.ok()) {
        if (oStatus.IsNotFound()) {
            //PLG1Debug("LevelDB.Get not found, instanceid %lu", llInstanceID);
            return 0;
        }

        //PLG1Err("LevelDB.Get fail, instanceid %lu", llInstanceID);
        return -1;
    }

    int ret = value_store_->ForceDel(sFileID, llInstanceID);
    if (ret != 0) {
        return ret;
    }

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = oWriteOptions.sync;

    oStatus = leveldb_->Delete(oLevelDBWriteOptions, sKey);
    if (!oStatus.ok()) {
        //PLG1Err("LevelDB.Delete fail, instanceid %lu", llInstanceID);
        return -1;
    }

    return 0;
}

int Database::Del(const WriteOptions & oWriteOptions, const uint64_t llInstanceID) {
    if (!has_init_) {
        //PLG1Err("no init yet");
        return -1;
    }

    std::string sKey = GenKey(llInstanceID);

    if (Util::FastRand() % 100 < 1) {
        //no need to del vfile every times.
        std::string sFileID;
        leveldb::Status oStatus = leveldb_->Get(leveldb::ReadOptions(), sKey, &sFileID);
        if (!oStatus.ok()) {
            if (oStatus.IsNotFound()) {
                //PLG1Debug("LevelDB.Get not found, instanceid %lu", llInstanceID);
                return 0;
            }

            //PLG1Err("LevelDB.Get fail, instanceid %lu", llInstanceID);
            return -1;
        }

        int ret = value_store_->Del(sFileID, llInstanceID);
        if (ret != 0) {
            return ret;
        }
    }

    leveldb::WriteOptions oLevelDBWriteOptions;
    oLevelDBWriteOptions.sync = oWriteOptions.sync;

    leveldb::Status oStatus = leveldb_->Delete(oLevelDBWriteOptions, sKey);
    if (!oStatus.ok()) {
        //PLG1Err("LevelDB.Delete fail, instanceid %lu", llInstanceID);
        return -1;
    }

    return 0;
}

int Database::GetMaxInstanceID(uint64_t & llInstanceID) {
    llInstanceID = MINCHOSEN_KEY;

    leveldb::Iterator * it = leveldb_->NewIterator(leveldb::ReadOptions());

    it->SeekToLast();

    while (it->Valid()) {
        llInstanceID = GetInstanceIDFromKey(it->key().ToString());
        if (llInstanceID == MINCHOSEN_KEY
                || llInstanceID == SYSTEMVARIABLES_KEY
                || llInstanceID == MASTERVARIABLES_KEY) {
            it->Prev();
        }
        else {
            delete it;
            return 0;
        }
    }

    delete it;
    return 1;
}

std::string Database::GenKey(const uint64_t llInstanceID) {
    std::string sKey;
    sKey.append((char *)&llInstanceID, sizeof(uint64_t));
    return sKey;
}

const uint64_t Database::GetInstanceIDFromKey(const std::string & sKey) {
    uint64_t llInstanceID = 0;
    memcpy(&llInstanceID, sKey.data(), sizeof(uint64_t));

    return llInstanceID;
}

int Database::GetMinChosenInstanceID(uint64_t & llMinInstanceID) {
    if (!has_init_) {
        //PLG1Err("no init yet");
        return -1;
    }

    static uint64_t llMinKey = MINCHOSEN_KEY;
    std::string sValue;
    int ret = GetFromLevelDb(llMinKey, sValue);
    if (ret != 0 && ret != 1) {
        //PLG1Err("fail, ret %d", ret);
        return ret;
    }

    if (ret == 1) {
        //PLG1Err("no min chosen instanceid");
        llMinInstanceID = 0;
        return 0;
    }

    //old version, minchonsenid store in logstore.
    //new version, minchonsenid directly store in leveldb.
    if (value_store_->IsValidFileId(sValue)) {
        ret = Get(llMinKey, sValue);
        if (ret != 0 && ret != 1) {
            //PLG1Err("Get from log store fail, ret %d", ret);
            return ret;
        }
    }

    if (sValue.size() != sizeof(uint64_t)) {
        //PLG1Err("fail, mininstanceid size wrong");
        return -2;
    }

    memcpy(&llMinInstanceID, sValue.data(), sizeof(uint64_t));

    //PLG1Imp("ok, min chosen instanceid %lu", llMinInstanceID);

    return 0;
}

int Database::SetMinChosenInstanceID(const WriteOptions & oWriteOptions, const uint64_t llMinInstanceID) {
    if (!has_init_) {
        //PLG1Err("no init yet");
        return -1;
    }

    static uint64_t llMinKey = MINCHOSEN_KEY;
    char sValue[sizeof(uint64_t)] = {0};
    memcpy(sValue, &llMinInstanceID, sizeof(uint64_t));

    int ret = PutToLevelDb(true, llMinKey, std::string(sValue, sizeof(uint64_t)));
    if (ret != 0) {
        return ret;
    }

    //PLG1Imp("ok, min chosen instanceid %lu", llMinInstanceID);

    return 0;
}


int Database::SetSystemVariables(const WriteOptions & oWriteOptions, const std::string & sBuffer) {
    static uint64_t llSystemVariablesKey = SYSTEMVARIABLES_KEY;
    return PutToLevelDb(true, llSystemVariablesKey, sBuffer);
}

int Database::GetSystemVariables(std::string & sBuffer) {
    static uint64_t llSystemVariablesKey = SYSTEMVARIABLES_KEY;
    return GetFromLevelDb(llSystemVariablesKey, sBuffer);
}

int Database::SetMasterVariables(const WriteOptions & oWriteOptions, const std::string & sBuffer) {
    static uint64_t llMasterVariablesKey = MASTERVARIABLES_KEY;
    return PutToLevelDb(true, llMasterVariablesKey, sBuffer);
}

int Database::GetMasterVariables(std::string & sBuffer) {
    static uint64_t llMasterVariablesKey = MASTERVARIABLES_KEY;
    return GetFromLevelDb(llMasterVariablesKey, sBuffer);
}

MultiDatabase::MultiDatabase() {
}

MultiDatabase::~MultiDatabase() {
    for (auto & poDB : vec_db_list_) {
        delete poDB;
    }
}

int MultiDatabase::Init(const std::string & sDBPath, const int iGroupCount) {
    if (access(sDBPath.c_str(), F_OK) == -1) {
        //PLErr("DBPath not exist or no limit to open, %s", sDBPath.c_str());
        return -1;
    }

    if (iGroupCount < 1 || iGroupCount > 100000) {
        //PLErr("Groupcount wrong %d", iGroupCount);
        return -2;
    }

    std::string sNewDBPath = sDBPath;

    if (sDBPath[sDBPath.size() - 1] != '/') {
        sNewDBPath += '/';
    }

    for (int iGroupIdx = 0; iGroupIdx < iGroupCount; iGroupIdx++) {
        char sGroupDBPath[512] = {0};
        snprintf(sGroupDBPath, sizeof(sGroupDBPath), "%sg%d", sNewDBPath.c_str(), iGroupIdx);

        Database * poDB = new Database();
        assert(poDB != nullptr);
        vec_db_list_.push_back(poDB);

        if (poDB->Init(sGroupDBPath, iGroupIdx) != 0) {
            return -1;
        }
    }

    //PLImp("OK, DBPath %s groupcount %d", sDBPath.c_str(), iGroupCount);

    return 0;
}

const std::string MultiDatabase::GetLogStorageDirPath(const int iGroupIdx) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return "";
    }

    return vec_db_list_[iGroupIdx]->GetDbPath();
}

int MultiDatabase::Get(const int iGroupIdx, const uint64_t llInstanceID, std::string & sValue) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->Get(llInstanceID, sValue);
}

int MultiDatabase::Put(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID, const std::string & sValue) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->Put(oWriteOptions, llInstanceID, sValue);
}

int MultiDatabase::Del(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->Del(oWriteOptions, llInstanceID);
}

int MultiDatabase::ForceDel(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llInstanceID) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->ForceDel(oWriteOptions, llInstanceID);
}

int MultiDatabase::GetMaxInstanceID(const int iGroupIdx, uint64_t & llInstanceID) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->GetMaxInstanceID(llInstanceID);
}

int MultiDatabase::SetMinChosenInstanceID(const WriteOptions & oWriteOptions, const int iGroupIdx, const uint64_t llMinInstanceID) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->SetMinChosenInstanceID(oWriteOptions, llMinInstanceID);
}

int MultiDatabase::GetMinChosenInstanceID(const int iGroupIdx, uint64_t & llMinInstanceID) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->GetMinChosenInstanceID(llMinInstanceID);
}

int MultiDatabase::ClearAllLog(const int iGroupIdx){
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->ClearAllLog();
}

int MultiDatabase::SetSystemVariables(const WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->SetSystemVariables(oWriteOptions, sBuffer);
}

int MultiDatabase::GetSystemVariables(const int iGroupIdx, std::string & sBuffer) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->GetSystemVariables(sBuffer);
}

int MultiDatabase::SetMasterVariables(const WriteOptions & oWriteOptions, const int iGroupIdx, const std::string & sBuffer) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->SetMasterVariables(oWriteOptions, sBuffer);
}

int MultiDatabase::GetMasterVariables(const int iGroupIdx, std::string & sBuffer) {
    if (iGroupIdx >= (int)vec_db_list_.size()) {
        return -2;
    }

    return vec_db_list_[iGroupIdx]->GetMasterVariables(sBuffer);
}

}

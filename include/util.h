#pragma once

#include <stdint.h>
#include <vector>

namespace paxos {

class Util {
public:
    static uint64_t GenGid(const uint64_t llNodeID);

    static const uint32_t FastRand();
};

class Time {
public:
    static const uint64_t GetTimestampMS();

    static const uint64_t GetSteadyClockMS();

    static void MsSleep(const int time_ms);
};

class FileUtils {
public:
    static int IsDir(const std::string & sPath, bool & bIsDir);

    static int DeleteDir(const std::string & sDirPath);

    static int IterDir(const std::string & sDirPath, std::vector<std::string> & vecFilePathList);
};

class TimeStat {
public:
    TimeStat();

    int Point();
private:
    uint64_t time_;
};


}

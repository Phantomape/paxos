#pragma once

#include <stdint.h>
#include <string>
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
    static int IsDir(const std::string& path, bool& is_dir);

    static int DeleteDir(const std::string& dir_path);

    static int IterDir(const std::string& dir_path, std::vector<std::string> & vec_file_path_list);
};

class TimeStat {
public:
    TimeStat();

    int Point();
private:
    uint64_t time_;
};


}

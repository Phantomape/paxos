#pragma once

#include <cstdint>

namespace paxos {

class Instance {
public:
    Instance();
    ~Instance();

    void Init();
    void Start();
    void Stop();

    const uint64_t GetCurrentInstanceId();
    const uint64_t GetMinInstanceId();

}

}
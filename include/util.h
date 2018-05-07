#pragma once

#include <stdint.h>

namespace paxos {

class Util {
public:
    static uint64_t GenGid(const uint64_t llNodeID);

    static const uint32_t FastRand();
};

}

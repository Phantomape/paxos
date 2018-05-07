#include "util.h"
#include <thread>

namespace paxos {

uint64_t Util::GenGid(const uint64_t llNodeID)
{
    return (llNodeID ^ FastRand()) + FastRand();
}

#ifdef __i386

__inline__ uint64_t rdtsc()
{
    uint64_t x;
    __asm__ volatile ("rdtsc" : "=A" (x));
    return x;
}

#elif __amd64

__inline__ uint64_t rdtsc()
{

    uint64_t a, d;
    __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
    return (d<<32) | a;
}

#endif

struct FastRandomSeed {
    bool init;
    unsigned int seed;
};

static __thread FastRandomSeed seed_thread_safe = { false, 0 };

static void ResetFastRandomSeed()
{
    seed_thread_safe.seed = rdtsc();
    seed_thread_safe.init = true; 
}

static void InitFastRandomSeedAtFork()
{
    pthread_atfork(ResetFastRandomSeed, ResetFastRandomSeed, ResetFastRandomSeed);
}

static void InitFastRandomSeed()
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, InitFastRandomSeedAtFork);

    ResetFastRandomSeed();
}

const uint32_t Util::FastRand()
{
    if (!seed_thread_safe.init)
    {
        InitFastRandomSeed();
    }

    return rand_r(&seed_thread_safe.seed);
}

}
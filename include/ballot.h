#pragma once

#include <cstdint>

namespace paxos {

class Ballot {
public:
    Ballot();
    Ballot(const uint64_t proposal_id, const uint64_t node_id_);
    ~Ballot();

    bool operator >= (const Ballot &other) const;
    bool operator != (const Ballot &other) const;
    bool operator == (const Ballot &other) const;
    bool operator > (const Ballot &other) const;

    void reset();

    uint64_t proposal_id_;
    uint64_t node_id_;
};

}
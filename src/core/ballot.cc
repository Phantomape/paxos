#include "ballot.h"

namespace paxos {

Ballot::Ballot() : proposal_id_(0), node_id_(0) {}

Ballot::Ballot(const uint64_t proposal_id, const uint64_t node_id) 
    : proposal_id_(proposal_id), node_id_(node_id) {}

Ballot::~Ballot() {}

bool Ballot::operator >= (const Ballot &other) const {
    if (proposal_id_ == other.proposal_id_) {
        return node_id_ >= other.node_id_;
    }
    else {
        return proposal_id_ >= other.proposal_id_;
    }
}

bool Ballot::operator != (const Ballot &other) const {
    return proposal_id_ != other.proposal_id_ 
        || node_id_ != other.node_id_;
}

bool Ballot::operator == (const Ballot &other) const {
    return proposal_id_ == other.proposal_id_
        && node_id_ == other.node_id_;
}

bool Ballot::operator > (const Ballot &other) const {
    if (proposal_id_ == other.proposal_id_) {
        return node_id_ > other.node_id_;
    }
    else {
        return proposal_id_ > other.proposal_id_;
    }
}

void Ballot::reset() {
    proposal_id_ = 0;
    node_id_ = 0;
}

}
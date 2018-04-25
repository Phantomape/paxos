#include "ballot.h"

namespace paxos {

Ballot::Ballot() : proposal_id_(0) {}

Ballot::~Ballot() {}

bool Ballot::operator >= (const Ballot &other) const {
    if (proposal_id_ == other.proposal_id_) {
        return true;
    }
    else {
        return proposal_id_ >= other.proposal_id_;
    }
}

bool Ballot::operator != (const Ballot &other) const {
    return proposal_id_ != other.proposal_id_;
}

bool Ballot::operator == (const Ballot &other) const {
    return proposal_id_ == other.proposal_id_;
}

bool Ballot::operator > (const Ballot &other) const {
    if (proposal_id_ == other.proposal_id_) {
        return true;
    }
    else {
        return proposal_id_ > other.proposal_id_;
    }
}

void Ballot::reset() {
    proposal_id_ = 0;
}

}
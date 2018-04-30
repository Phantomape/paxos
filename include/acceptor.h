#pragma once

#include "base.h"

namespace paxos {

class Acceptor : public Base {
public: 
    Acceptor();
    ~Acceptor();

    int Init();
    virtual void InitInstance();

    const Ballot& GeteAcceptedBallot() const;
    const Ballot& GetPromiseBallot() const;

    const std::string& GetAcceptedValue();
    void SetAcceptedValue(const std::string& accepted_val);

    void SetAcceptedBallot(const Ballot& accepted_ballot);
    void SetPromiseBallot(const Ballot& promised_ballot);

    void OnPrepare(const PaxosMsg& recv_paxos_msg);
    void OnAccept(const PaxosMsg& paxos_msg);
private:
    Ballot promised_ballot;
    Ballot accepted_ballot;
    std::string accepted_val_;
};

}
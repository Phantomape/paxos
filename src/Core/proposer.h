#pragma once

#include "base.h"

namespace paxos {

class Proposer : public Base {
public:
    Proposer();
    ~Proposer();

    void Accept();
    void Prepare();

    void ExitPrepare();
    void ExitAccept();

    void OnAccept();
    void OnAcceptRejected();
    void OnAcceptTimeout();
    void OnPrepare();
    void OnPrepareRejected();
    void OnPrepareTimeout();
};

}
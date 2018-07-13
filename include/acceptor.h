#pragma once

#include "base.h"
#include "config.h"
#include "paxos_log.h"

namespace paxos {

class Acceptor : public Base {
public: 
    Acceptor(
            const Config * config, 
            const Communicate * communicate, 
            const Instance * instance,
            const LogStorage * log_storage
            );

    ~Acceptor();

    int Init();

    virtual void InitInstance();

    const Ballot& GetAcceptedBallot() const;

    const Ballot& GetPromiseBallot() const;

    const std::string& GetAcceptedValue();

    int Load(uint64_t & instance_id);

    const uint32_t GetChecksum() const;

    void SetAcceptedValue(const std::string& accepted_val);

    void SetAcceptedBallot(const Ballot& accepted_ballot);

    void SetPromiseBallot(const Ballot& promised_ballot);

    int OnPrepare(const PaxosMsg& recv_paxos_msg);

    void OnAccept(const PaxosMsg& paxos_msg);

private:
    Ballot promised_ballot_;
    Ballot accepted_ballot_;
    std::string accepted_val_;
    uint32_t checksum_;
    Config* config_;
    PaxosLog paxos_log_;

    int sync_times_;
};

}
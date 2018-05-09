#pragma once

#include "state_machine.h"
#include <string>

namespace paxos {

class InternalStateMachine : public StateMachine {
public:
    virtual ~InternalStateMachine() {}

    virtual int GetCheckpointBuffer(std::string & sCPBuffer) = 0;

    virtual int UpdateByCheckpoint(const std::string & sCPBuffer, bool & bChange) = 0;
};
    
}

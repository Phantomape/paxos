#include "ioloop.h"

namespace paxos {

IoLoop::IoLoop() {}

IoLoop::~IoLoop() {}

void IoLoop::Run() {
    is_end_ = false;
    is_start_ = true;
    while (true) {
        int next_timeout = 1000;
        // Deal with time out
        Init(next_timeout);
        if (is_end_) {
            break;
        }
    }    
}


}
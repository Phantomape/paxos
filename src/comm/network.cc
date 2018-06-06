#include "network.h"
#include "node.h"

namespace paxos {

Network::Network() : node(nullptr) {}

int Network::OnReceiveMessage(const char* message, const int message_len) {
    if (node != nullptr) {
        // node->OnReceiveMessage(message, message_len);
    }
    else {
    }

    return 0;
}

}
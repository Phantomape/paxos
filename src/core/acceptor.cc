#include "acceptor.h"
#include <iostream>
#include <string>

namespace paxos {
    
Acceptor::Acceptor() {
    std::cout << "Acceptor::Acceptor()" << std::endl;
}

Acceptor::~Acceptor() {
    std::cout << "Acceptor::~Acceptor()" << std::endl;
}


void Acceptor::OnPrepare(const PaxosMsg &recv_paxos_msg) {
    std::cout << "Acceptor::OnPrepare()" << std::endl;

    PaxosMsg send_paxos_msg;
    send_paxos_msg.set_msgtype(1); // Replace thie hard-coded number

}


}


#include "default_network.h"
#include "udp.h"

namespace paxos {

DefaultNetwork::DefaultNetwork() : udp_recv_(this), tcp_io_thread_(this) {
}

DefaultNetwork::~DefaultNetwork() {
}

void DefaultNetwork::Stop() {
    udp_recv_.Stop();
    udp_send_.Stop();
    tcp_io_thread_.Stop();
}

int DefaultNetwork::Init(const std::string & ip_listened, const int port_listened, const int io_thread_count) {
    int res = udp_send_.Init();
    if (res != 0) {
        return res;
    }

    res = udp_recv_.Init(port_listened);
    if (res != 0) {
        return res;
    }

    res = tcp_io_thread_.Init(ip_listened, port_listened, io_thread_count);
    if (res != 0) {
        return res;
    }

    return 0;
}

void DefaultNetwork::Run() {
    udp_send_.Start();
    udp_recv_.Start();
    tcp_io_thread_.Start();
}

int DefaultNetwork::SendMessageTCP(const int group_idx, const std::string & ip, const int port, const std::string & sMessage) {
    return tcp_io_thread_.AddMessage(group_idx, ip, port, sMessage);
}

int DefaultNetwork::SendMessageUDP(const int group_idx, const std::string & ip, const int port, const std::string & sMessage) {
    return udp_send_.AddMessage(ip, port, sMessage);
}

}



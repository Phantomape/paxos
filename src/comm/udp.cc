#include "default_network.h"
#include "udp.h"
#include <arpa/inet.h>
#include <cstring>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace paxos {

UdpRecv::UdpRecv(DefaultNetwork* default_network) 
    : default_network(default_network), socket_file_descriptor_(-1), is_ended_(false), is_started_(false)
{

}

UdpRecv::~UdpRecv() {
    if (socket_file_descriptor_ != -1) {
        close(socket_file_descriptor_);
        socket_file_descriptor_ = -1;
    }
}

void UdpRecv::Stop() {
    if (is_started_) {
        is_ended_ = true;
        Join();
    }
}

int UdpRecv::Init(const int port) {
    if ((socket_file_descriptor_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int enable = 1;
    setsockopt(socket_file_descriptor_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(socket_file_descriptor_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return -1;
    }

    return 0;
}

void UdpRecv::Run() {
    is_started_ = true;

    char buf[65536] = {0};

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    std::memset(&addr, 0, sizeof(addr));

    while (true) {
        if (is_ended_) {
            return;
        }

        struct pollfd fd;
        int res;

        fd.fd = socket_file_descriptor_;
        fd.events = POLLIN;
        res = poll(&fd, 1, 500);

        if (res == 0 || res == -1) {
            continue;
        }

        int recv_len = recvfrom(socket_file_descriptor_, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
        if (recv_len > 0) {
            // default_network->OnReceiveMessage(buf, recv_len);
        }
    }
}

UdpSend::UdpSend() : socket_file_descriptor_(-1), is_ended_(false), is_started_(false) {}

UdpSend::~UdpSend() {
    while (!send_queue.empty()) {
        Packet* packet = send_queue.peek();
        send_queue.pop();
        delete packet;
    }
}

int UdpSend::Init() {
    if ((socket_file_descriptor_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }
    return 0;
}

void UdpSend::Stop() {
    if (is_started_) {
        is_ended_ = true;
        Join();
    }
}

void UdpSend::SendMessage(const std::string& ip, const int port, const std::string& message) {
    struct sockaddr_in addr;
    int addr_len = sizeof(struct sockaddr_in);
    std::memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    int res = sendto(socket_file_descriptor_, message.data(), (int)message.size(), 0, (struct sockaddr*)&addr, addr_len);
    if (res > 0) {

    }
}

void UdpSend::Run() {
    is_started_ = true;

    while (true) {
        Packet* packet = nullptr;

        send_queue.lock();

        bool is_succeeded = send_queue.peek(packet, 1000);
        if (is_succeeded) {
            send_queue.pop();
        }

        send_queue.unlock();

        if (packet != nullptr) {
            SendMessage(packet->ip, packet->port, packet->message);
            delete packet;
        }

        if (is_ended_) {
            return;
        }
    }
}

int UdpSend::AddMessage(const std::string& ip, const int port, const std::string& message) {
    send_queue.lock();
    
    int UDP_QUEUE_MAX_LEN = 100; // This const need to be stored somewhere else
    if ((int)send_queue.size() > UDP_QUEUE_MAX_LEN) {
        send_queue.unlock();

        return -2;
    }

    Packet* packet = new Packet;
    packet->ip = ip;
    packet->port = port;
    packet->message = message;

    send_queue.add(packet);
    send_queue.unlock();

    return 0;
}

}
#include "socket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace paxos {

template <class T>
string str(const T& t) { 
    ostringstream os;
    os << t;
    return os.str();
}

SocketAddress::SocketAddress() {
    addr_.addr.sa_family = AF_INET;
    addr_.in.sin_port = 0;
    addr_.in.sin_addr.s_addr = htonl(INADDR_NONE);
}

SocketAddress::SocketAddress(unsigned short port) {
    SetAddress(port);
}

SocketAddress::SocketAddress(const std::string& addr) {
    SetAddress(addr);
}

SocketAddress::SocketAddress(const std::string& addr, unsigned short port) {
    SetAddress(addr, port);
}

SocketAddress::SocketAddress(const Addr& addr) {
    SetAddress(addr);
}

SocketAddress::SocketAddress(const sockaddr_in& addr) {
    SetAddress(addr);
}

SocketAddress::SocketAddress(const sockaddr_un& addr) {
    SetAddress(addr);
}

void SocketAddress::SetAddress(unsigned short port) {
    addr_.addr.sa_family = AF_INET;
    addr_.in.sin_port = htons(port);
    addr_.in.sin_addr.s_addr = htonl(INADDR_ANY);
}

void SocketAddress::SetAddress(const std::string& addr) {
    std::string::size_type pos = addr.find_last_of(":");
    if (pos == std::string::npos || pos == addr.size() - 1) {
        SetAddress(addr, 0);
    } else {
        std::string port = addr.substr(pos + 1);
        SetAddress(addr.substr(0, pos), (unsigned short)atoi(port.c_str()));
    }
}

void SocketAddress::SetAddress(const std::string& addr, unsigned short port) {
    unsigned long ip = inet_addr(addr.c_str());
    if (ip == static_cast<unsigned long>(INADDR_NONE)) {
        throw SocketException("inet_addr error \"" + addr + "\"");
    }

    addr_.addr.sa_family = AF_INET;
    addr_.in.sin_port = htons(port);
    addr_.in.sin_addr.s_addr = ip;
}

void SocketAddress::SetUnixDomain(const std::string& path) {
    if (path.size() + 1 > sizeof(addr_.un.sun_path)) {
        throw SocketException("unix domain path \"" + path + "\" too long");
    }

    addr_.addr.sa_family = AF_UNIX;
    strcpy(addr_.un.sun_path, path.c_str());
}

unsigned long SocketAddress::GetIp() const {
    return addr_.in.sin_addr.s_addr;
}

unsigned short SocketAddress::GetPort() const {
    return ntohs(addr_.in.sin_port);
}

void SocketAddress::GetAddress(Addr& addr) const {
    memcpy(&addr, &addr_, sizeof(addr));
}

void SocketAddress::GetAddress(sockaddr_in& addr) const {
    memcpy(&addr, &addr_.in, sizeof(addr));
}

void SocketAddress::GetAddress(sockaddr_un& addr) const {
    memcpy(&addr, &addr_.un, sizeof(addr));
}

void SocketAddress::SetAddress(const Addr& addr) {
    memcpy(&addr_, &addr, sizeof(addr));
}

void SocketAddress::SetAddress(const sockaddr_in& addr) {
    memcpy(&addr_.in, &addr, sizeof(addr));
    addr_.addr.sa_family = AF_INET;
}

void SocketAddress::SetAddress(const sockaddr_un& addr) {
    memcpy(&addr_.un, &addr, sizeof(addr));
}

int SocketAddress::GetFamily() const {
    return addr_.addr.sa_family;
}

socklen_t SocketAddress::GetAddressLength(const Addr& addr) {
    if (addr.addr.sa_family == AF_INET) {
        return sizeof(addr.in);
    } else if (addr.addr.sa_family == AF_UNIX || addr.addr.sa_family == AF_LOCAL) {
        return sizeof(addr.un);
    }
    return sizeof(addr);
}

std::string SocketAddress::GetHost() const {
    if (addr_.addr.sa_family == AF_UNIX || addr_.addr.sa_family == AF_LOCAL) {
        return addr_.un.sun_path;
    } else {
        char buff[16];

        if (inet_ntop(AF_INET, &addr_.in.sin_addr, buff, sizeof(buff)) == 0) {
            return std::string();
        } else {
            return buff;
        }
    }
}

std::string SocketAddress::ToString() const {
    if (addr_.addr.sa_family == AF_UNIX || addr_.addr.sa_family == AF_LOCAL) {
        return addr_.un.sun_path;
    }
    return GetHost() + ":" + str(ntohs(addr_.in.sin_port));
}

bool SocketAddress::operator ==(const SocketAddress& addr) const {
    if (addr_.addr.sa_family != addr.addr_.addr.sa_family) {
        return false;
    } else if (addr_.addr.sa_family == AF_UNIX || addr_.addr.sa_family == AF_LOCAL) {
        return strcmp(addr_.un.sun_path, addr.addr_.un.sun_path) == 0;
    } else {
        return addr_.in.sin_addr.s_addr == addr.addr_.in.sin_addr.s_addr
                && addr_.in.sin_port == addr.addr_.in.sin_port;
    }
}

SocketBase::SocketBase() : family_(AF_INET), handle_(-1) {}

SocketBase::SocketBase(int family, int handle) : family_(family), handle_(handle) {
    if (handle == -1) {
        InitHandle(family);
    }
}

SocketBase::~SocketBase() {
    if (handle_ != -1) {
        ::close(handle_);
        handle_ = -1;
    }
}

void SocketBase::InitHandle(int family) {
    handle_ = ::socket(family, SOCK_STREAM, 0);
    if (handle_ == -1) {
        throw SocketException("socket error");
    }
}

int SocketBase::GetFamily() const {
    return family_;
}

int SocketBase::GetSocketHandle() const {
    return handle_;
}

void SocketBase::SetSocketHandle(int handle, int family) {
    if (handle_ != handle) {
        Close();

        handle_ = handle;
        family_ = family;
    }
}

int SocketBase::DetachSocketHandle() {
    int handle = handle_;
    handle_ = -1;
    family_ = AF_INET;
    return handle;
}

bool SocketBase::GetNonBlocking() const {
    return GetNonBlocking(handle_);
}

void SocketBase::SetNonBlocking(bool on) {
    SetNonBlocking(handle_, on);
}

bool SocketBase::GetNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return flags & O_NONBLOCK;
}

void SocketBase::SetNonBlocking(int fd, bool on) {
    int flags = ::fcntl(fd, F_GETFL, 0);

    if (on) {
        if (flags & O_NONBLOCK) {
            return;
        }
        flags |= O_NONBLOCK;
    } else {
        if (!(flags & O_NONBLOCK)) {
            return;
        }
        flags &= ~O_NONBLOCK;
    }

    if (0 != ::fcntl(fd, F_SETFL, flags)) {
        ;
    }
}

socklen_t SocketBase::GetOption(int level, int option, void* value, socklen_t optLen) const {
    if (::getsockopt(handle_, level, option, static_cast<char*>(value), &optLen) == -1) {
        throw SocketException("getsockopt error");
    }
    return optLen;
}

void SocketBase::SetOption(int level, int option, void* value, socklen_t optLen) const {
    if (::setsockopt(handle_, level, option, static_cast<char*>(value), optLen) == -1) {
        throw SocketException("setsockopt error");
    }
}

void SocketBase::Close() {
    if (handle_ != -1) {
        if (::close(handle_) == -1) {
            handle_ = -1;
            throw SocketException("close error");
        } else {
            handle_ = -1;
        }
    }
}

void SocketBase::Reset() {
    Close();
    InitHandle(family_);
}

}
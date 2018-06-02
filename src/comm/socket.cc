#include "socket.h"
#include <fcntl.h>
#include <sstream>
#include <netinet/tcp.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace paxos {

template <class T>
std::string str(const T& t) {
    std::ostringstream os;
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

const static int SOCKET_BUF_SIZE = 512;

Socket::Socket() : SocketBase(AF_INET, -1) {}

Socket::Socket(int handle) : SocketBase(AF_INET, handle)  {}

Socket::Socket(const SocketAddress& addr) {
    Connect(addr);
}

Socket::Socket(int family, int handle) : SocketBase(family, handle) {}

Socket::~Socket() {}

int Socket::GetSendTimeout() const {
    timeval tv;
    GetOption(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(timeval));
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void Socket::SetSendTimeout(int timeout) {
    if (timeout >= 0) {
        timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        SetOption(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    } else {
        SetOption(SOL_SOCKET, SO_SNDTIMEO, 0, sizeof(int));
    }
}

int Socket::GetReceiveTimeout() const {
    timeval tv;
    GetOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval));
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void Socket::SetReceiveTimeout(int timeout) {
    if (timeout >= 0) {
        timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        SetOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    } else {
        SetOption(SOL_SOCKET, SO_RCVTIMEO, 0, sizeof(int));
    }
}

void Socket::SetSendBufferSize(int size) {
    SetOption(SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

void Socket::SetReceiveBufferSize(int size) {
    SetOption(SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

void Socket::SetQuickAck(bool on) {
    int v = on ? 1 : 0;
    SetOption(IPPROTO_TCP, TCP_QUICKACK, &v, sizeof(v));
}

void Socket::SetNoDelay(bool on) {
    int v = on ? 1 : 0;
    SetOption(IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
}

SocketAddress Socket::GetRemoteAddress() const {
    return GetRemoteAddress(handle_);
}

SocketAddress Socket::GetRemoteAddress(int fd) {
    SocketAddress::Addr addr;
    socklen_t len = sizeof(addr);

    SocketAddress remoteAddr;
    if (::getpeername(fd, &addr.addr, &len) == 0) {
        remoteAddr.SetAddress(addr);
    }

    return remoteAddr;
}

SocketAddress Socket::GetLocalAddress() const {
    return GetLocalAddress(handle_);
}

SocketAddress Socket::GetLocalAddress(int fd) {
    SocketAddress::Addr addr;
    socklen_t len = sizeof(addr);

    SocketAddress localAddr;
    if (::getsockname(fd, &addr.addr, &len) == 0) {
        localAddr.SetAddress(addr);
    }

    return localAddr;
}

void Socket::Connect(const SocketAddress& addr) {
    SocketAddress::Addr sockAddr;
    addr.GetAddress(sockAddr);

    if (handle_ == -1 || family_ != addr.GetFamily()) {
        Close();
        InitHandle(addr.GetFamily());
    }

    if (handle_ < 0) {
        throw SocketException("bad handle");
    }

    if (::connect(handle_, &sockAddr.addr, SocketAddress::GetAddressLength(sockAddr)) == -1) {
        if (errno != EINPROGRESS) {
            std::string msg = "connect " + addr.ToString() + " error";
            throw SocketException(msg);
        } else if (!GetNonBlocking()) {
            std::string msg = "connect " + addr.ToString() + " timeout";
            throw SocketException(msg);
        }
    }
}

int Socket::Send(const char* data, int dataSize, bool* again) {
    const char* p = data;
    int n = 0;

    if (again) {
        *again = false;
    }

    while (dataSize > 0) {
        n = ::send(handle_, p, dataSize, 0);
        if (n > 0) {
            p += n;
            dataSize -= n;
        } else if (errno == EAGAIN) {
            if (again) {
                *again = true;
            }

            if (!GetNonBlocking()) {
                throw SocketException("send timeout");
            }

            break;
        } else if (errno == EINTR) {
            continue;
        } else {
            throw SocketException("send error");
        }
    }

    return p - data;
}

int Socket::Receive(char* buffer, int bufferSize, bool* again) {
    char* p = buffer;
    int n = 0;

    if (again) {
        *again = false;
    }

    while (bufferSize > 0) {
        n = ::recv(handle_, p, bufferSize, 0);
        if (n > 0) {
            p += n;

            if (n < bufferSize) {
                break;
            }

            bufferSize -= n;
        } else if (n == 0) {
            break;
        } else if (errno == EAGAIN) {
            if (again) {
                *again = true;
            }

            if (!GetNonBlocking()) {
                throw SocketException("recv timeout");
            }

            break;
        } else if (errno == EINTR) {
            continue;
        } else {
            throw SocketException("recv error");
        }
    }

    return p - buffer;
}

void Socket::ShutdownInput() {
    if (::shutdown(handle_, SHUT_RD) == -1) {
        throw SocketException("shutdown(SHUT_RD) error");
    }
}

void Socket::ShutdownOutput() {
    if (::shutdown(handle_, SHUT_WR) == -1) {
        throw SocketException("shutdown(SHUT_WR) error");
    }
}

void Socket::Shutdown() {
    if (::shutdown(handle_, SHUT_RDWR) == -1) {
        throw SocketException("shutdown error");
    }
}

ServerSocket::ServerSocket() {}

ServerSocket::ServerSocket(const SocketAddress& addr) : SocketBase(addr.GetFamily(), -1) {
    Listen(addr);
}

ServerSocket::~ServerSocket() {}

void ServerSocket::Listen(const SocketAddress& addr, int backlog) {
    SocketAddress::Addr localAddr;
    addr.GetAddress(localAddr);

    if (handle_ == -1 || family_ != addr.GetFamily()) {
        Close();
        InitHandle(addr.GetFamily());
    }

    if (handle_ < 0) {
        throw SocketException("bad handle");
    }

    if (addr.GetFamily() == AF_UNIX) {
        ::unlink(localAddr.un.sun_path);
    } else {
        int reuse = 1;
        SetOption(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    }

    if (::bind(handle_, &localAddr.addr, SocketAddress::GetAddressLength(localAddr)) == -1) {
        throw SocketException("bind error");
    }

    if (::listen(handle_, backlog) == -1) {
        throw SocketException("listen error");
    }
}

int ServerSocket::GetAcceptTimeout() const {
    timeval tv;
    GetOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval));
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void ServerSocket::SetAcceptTimeout(int timeout) {
    timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    SetOption(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

Socket* ServerSocket::Accept() {
    int fd = AcceptFd(NULL);
    if (fd >= 0) {
        return new Socket(family_, fd);
    } else {
        return NULL;
    }
}

int ServerSocket::AcceptFd(SocketAddress* addr) {
    SocketAddress::Addr a;
    socklen_t n = sizeof(a);

    int fd = ::accept(handle_, &a.addr, &n);
    if (fd == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
        throw SocketException("accept error");
    } else if (fd >= 0 && addr) {
        addr->SetAddress(a);
    }
    return fd;
}

}
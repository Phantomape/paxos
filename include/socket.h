#pragma once

#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string>

namespace paxos {

class SocketAddress {
public:
    enum Type {
        TYPE_LOOPBACK = 1, 
        TYPE_INNER = 2, 
        TYPE_OUTER = 3 
    };

    union Addr {
        sockaddr addr;
        sockaddr_in in;
        sockaddr_un un;
    };

    SocketAddress();

    SocketAddress(unsigned short port);

    SocketAddress(const std::string& addr);

    SocketAddress(const std::string& addr, unsigned short port);

    SocketAddress(const Addr& addr);

    SocketAddress(const sockaddr_in& addr);

    SocketAddress(const sockaddr_un& addr);

    void GetAddress(Addr& addr) const;

    void GetAddress(sockaddr_in& addr) const;

    void GetAddress(sockaddr_un& addr) const;

    static socklen_t GetAddressLength(const Addr& addr);

    static Type GetAddressType(const std::string& ip);

    int GetFamily() const;

    std::string GetHost() const;

    unsigned long GetIp() const;

    unsigned short GetPort() const;

    void SetAddress(unsigned short port);

    void SetAddress(const std::string& addr);

    void SetAddress(const std::string& addr, unsigned short port);

    void SetAddress(const Addr& addr);

    void SetAddress(const sockaddr_in& addr);

    void SetAddress(const sockaddr_un& addr);

    void SetUnixDomain(const std::string& path);

    std::string ToString() const;

    bool operator ==(const SocketAddress& addr) const;

private:
    Addr addr_;
};

class SocketBase {
public:
    SocketBase();

    SocketBase(int family, int handle);

    virtual ~SocketBase();

    virtual int DetachSocketHandle();
    
    virtual int GetFamily() const;
    
    static bool GetNonBlocking(int fd);
    
    virtual bool GetNonBlocking() const;

    virtual int GetSocketHandle() const;

    virtual void SetNonBlocking(bool on);

    static void SetNonBlocking(int fd, bool on);

    virtual void SetSocketHandle(int handle, int family = AF_INET);
    
    virtual void Close();

    virtual void ReSet();

protected:  

    void InitHandle(int family);

    socklen_t GetOption(int level, int option, void* value, socklen_t optLen) const;

    void SetOption(int level, int option, void* value, socklen_t optLen) const;

    int family_;
    int handle_;
};

class Socket : public SocketBase {
public:
    Socket();

    Socket(const SocketAddress& addr);

    Socket(int handle);

    Socket(int family, int handle);

    virtual ~Socket();

    SocketAddress GetLocalAddress() const;

    static SocketAddress GetLocalAddress(int fd);

    int GetReceiveTimeout() const;

    SocketAddress GetRemoteAddress() const;

    static SocketAddress GetRemoteAddress(int fd);

    int GetSendTimeout() const;

    void SetNoDelay(bool on);

    void SetQuickAck(bool on);

    void SetReceiveBufferSize(int size);

    void SetReceiveTimeout(int timeout);

    void SetSendBufferSize(int size);

    void SetSendTimeout(int timeout);

    virtual void Connect(const SocketAddress& addr);

    virtual int Send(const char* data, int dataSize, bool* again = 0);

    virtual int Receive(char* buffer, int bufferSize, bool* again = 0);

    virtual void ShutdownInput();

    virtual void ShutdownOutput();

    virtual void Shutdown();
};

class ServerSocket : public SocketBase {
public:
    ServerSocket();

    ServerSocket(const SocketAddress& addr);

    virtual ~ServerSocket();

    int GetAcceptTimeout() const;

    void SetAcceptTimeout(int timeout);

    virtual void listen(const SocketAddress& addr, int backlog = SOMAXCONN);

    virtual Socket* accept();

    virtual int acceptfd(SocketAddress* addr);
};

class SocketException : public SysCallException {
public:
    SocketException(const std::string& err_msg, bool detail = true)
        : SysCallException(errno, err_msg, detail) {}

    virtual ~SocketException() throw () {}
};

} 


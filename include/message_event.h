#pragma once

#include "event.h"
#include "socket.h"
#include <mutex>
#include <queue>

namespace paxos {

class EventLoop;
class Network;

enum MessageEventType {
    MessageEventType_RECV = 1,
    MessageEventType_SEND = 2,
};

enum MessageEventTimerType {
    MessageEventTimerType_Reconnect = 1,
};

class MessageEvent : public Event {
public: 
    MessageEvent(
        const int type, 
        const int fd, 
        const SocketAddress& addr,
        EventLoop* event_loop,
        Network* network
        );
    ~MessageEvent();

    int AddMessage(const std::string & message);

    int GetSocketFd() const;
    
    const std::string & GetSocketHost();

    int OnRead();

    int OnWrite();

    void OnTimeout(const uint32_t timer_id, const int type);

    void OnError(bool & need_delete);

    void OpenWrite();

    const bool IsActive();

private:
    int ReadLeft();

    //void ReadDone(BytesBuffer & oBytesBuffer, const int iLen);
    
    int WriteLeft();

    void WriteDone();

    int DoOnWrite();

    void ReConnect();

    Socket socket_;    
    SocketAddress addr_;
    std::string host_;
    Network* network_;
    int type_;

    char read_head_buffer_[sizeof(int)];
    int last_read_head_pos_;
    //BytesBuffer m_oReadCacheBuffer;
    int last_read_pos_;
    int left_read_len_;

    //BytesBuffer m_oWriteCacheBuffer;
    int last_write_pos_;
    int left_write_len_;

    struct QueueData
    {
        uint64_t llEnqueueAbsTime;
        std::string * psValue;
    };    

    std::queue<QueueData> m_oInQueue;
    int queue_size_;
    std::mutex mutex_;

    uint64_t last_active_time_;

    uint32_t reconnect_timeout_id_;
};

}

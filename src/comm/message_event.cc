#include "internal_options.h"
#include "message_event.h"
#include "network.h"
#include "event_loop.h"
#include "util.h"

namespace paxos {

MessageEvent::MessageEvent(
        const int type,
        const int fd, 
        const SocketAddress & addr, 
        EventLoop* event_loop,
        Network* network
        ) : Event(event_loop), socket_(fd), addr_(addr), network_(network) {
    type_ = type;

    left_read_len_ = 0;
    last_read_pos_ = 0;

    left_write_len_ = 0;
    last_write_pos_ = 0;

    // memset(m_sReadHeadBuffer, 0, sizeof(m_sReadHeadBuffer));
    last_read_head_pos_ = 0;

    socket_.SetNonBlocking(true);
    socket_.SetNoDelay(true);

    reconnect_timeout_id_ = 0;

    // last_active_time_ = Time::GetSteadyClockMS();
    host_ = addr_.GetHost();

    queue_size_ = 0;
}

MessageEvent::~MessageEvent() {
    while (!m_oInQueue.empty()) {
        QueueData tData = m_oInQueue.front();
        m_oInQueue.pop();

        delete tData.psValue;
    }
}

int MessageEvent::GetSocketFd() const {
    return socket_.GetSocketHandle();
}

const std::string & MessageEvent::GetSocketHost() {
    return host_;
}

const bool MessageEvent::IsActive() {
    uint64_t llNowTime = Time::GetSteadyClockMS();
    if (llNowTime > last_active_time_
            && ((int)(llNowTime - last_active_time_) > CONNECTION_NONACTIVE_TIMEOUT)) {
        return false;
    }

    return true;
}

int MessageEvent::AddMessage(const std::string & message) {
    /*
    last_active_time_ = Time::GetSteadyClockMS();
    std::unique_lock<std::mutex> oLock(mutex_);

    if ((int)m_oInQueue.size() > TCP_QUEUE_MAXLEN)
    {
        BP->GetNetworkBP()->TcpQueueFull();
        //PLErr("queue length %d too long, can't enqueue", m_oInQueue.size());
        return -2;
    }

    if (queue_size_ > MAX_QUEUE_MEM_SIZE)
    {
        //PLErr("queue memsize %d too large, can't enqueue", queue_size_);
        return -2;
    }

    QueueData tData;
    tData.llEnqueueAbsTime = Time::GetSteadyClockMS();
    tData.psValue = new string(sMessage);
    m_oInQueue.push(tData);

    queue_size_ += sMessage.size();

    oLock.unlock();

    JumpoutEpollWait();

    */
    return 0;
}

/*
void MessageEvent::ReadDone(BytesBuffer & oBytesBuffer, const int iLen)
{
    //PLHead("ok, len %d", iLen);
    network_->OnReceiveMessage(oBytesBuffer.GetPtr(), iLen);

    BP->GetNetworkBP()->TcpReadOneMessageOk(iLen);
}
*/

int MessageEvent::ReadLeft() {
    /*
    bool bAgain = false;
    int iReadLen = socket_.Receive(m_oReadCacheBuffer.GetPtr() + last_read_pos_, left_read_len_, &bAgain);
    //PLImp("readlen %d", iReadLen);
    if (iReadLen == 0)
    {
        //socket broken
        return -1;
    }

    left_read_len_ -= iReadLen;
    last_read_pos_ += iReadLen;

    if (left_read_len_ == 0)
    {
        ReadDone(m_oReadCacheBuffer, last_read_pos_);
        left_read_len_ = 0;
        last_read_pos_ = 0;
    }

    return 0;
    */
}

int MessageEvent::OnRead() {
    /*
    if (left_read_len_ > 0)
    {
        return ReadLeft();
    }
    
    int iReadLen = socket_.receive(m_sReadHeadBuffer + last_read_head_pos_, sizeof(int) - last_read_head_pos_);
    if (iReadLen == 0)
    {
        BP->GetNetworkBP()->TcpOnReadMessageLenError();
           PLErr("read head fail, readlen %d, socket broken", iReadLen);
        return -1;
    }

    last_read_head_pos_ += iReadLen;
    if (last_read_head_pos_ < (int)sizeof(int))
    {
        PLImp("head read pos %d small than sizeof(int) %zu", last_read_head_pos_, sizeof(int));
        return 0;
    }
    
    last_read_head_pos_ = 0;
    int niLen = 0;
    int iLen = 0;
    memcpy((char *)&niLen, m_sReadHeadBuffer, sizeof(int));
    iLen = ntohl(niLen) - 4;
    
    if (iLen < 0 || iLen > MAX_VALUE_SIZE)
    {
        PLErr("need to read len wrong %d", iLen);
        return -2;
    }

    m_oReadCacheBuffer.Ready(iLen);

    left_read_len_ = iLen;
    last_read_pos_ = 0;
    
    //second read maybe no data read, so readlen == 0 is ok.
    bool bAgain = false;
    iReadLen = socket_.receive(m_oReadCacheBuffer.GetPtr(), iLen, &bAgain);
    if (iReadLen == 0)
    {
        if (!bAgain)
        {
            PLErr("second read data fail, readlen %d, no again, socket broken", iReadLen);
            return -1;
        }
        else
        {
            PLErr("second read data, readlen %d need again", iReadLen);
            return 0;
        }
    }

    if (iReadLen == iLen)
    {
        ReadDone(m_oReadCacheBuffer, iLen);
        left_read_len_ = 0;
        last_read_pos_ = 0;
    }
    else if (iReadLen < iLen)
    {
        last_read_pos_ = iReadLen;
        left_read_len_ = iLen - iReadLen;

        PLImp("read buflen %d small than except len %d", iReadLen, iLen);
    }
    else
    {
        PLErr("read buflen %d large than except len %d", iReadLen, iLen);
        return -2;
    }
    */
    return 0;
}

void MessageEvent::OpenWrite() {
    if (!m_oInQueue.empty()) {
        if (IsDestroy()) {
            ReConnect();
            is_destroried_ = false;
        }
        else {
            AddEvent(EPOLLOUT);
        }
    }
}

void MessageEvent::WriteDone() {
    RemoveEvent(EPOLLOUT);
}

int MessageEvent::WriteLeft() {
    /*
    int iWriteLen = socket_.send(m_oWriteCacheBuffer.GetPtr() + last_write_pos_, left_write_len_);
    //PLImp("writelen %d", iWriteLen);
    if (iWriteLen < 0)
    {
        return -1;
    }

    if (iWriteLen == 0)
    {
        //no buffer to write, wait next epoll_wait
        //need wait next write
        AddEvent(EPOLLOUT);

        return 1;
    }

    left_write_len_ -= iWriteLen;
    last_write_pos_ += iWriteLen;

    if (left_write_len_ == 0)
    {
        left_write_len_ = 0;
        last_write_pos_ = 0;
    }
    */
    return 0;
}

int MessageEvent::OnWrite() {
    int ret = 0;
    while (!m_oInQueue.empty() || left_write_len_ > 0) {
        ret = DoOnWrite();
        if (ret != 0 && ret != 1) {
            return ret;
        }
        else if (ret == 1) {
            //need break, wait next write
            return 0;
        }
    }

    WriteDone();

    return 0;
}

int MessageEvent::DoOnWrite() {
    if (left_write_len_ > 0) {
        return WriteLeft();
    }

    mutex_.lock();
    if (m_oInQueue.empty()) {
        mutex_.unlock();
        return 0;
    }
    QueueData tData = m_oInQueue.front();
    m_oInQueue.pop();
    queue_size_ -= tData.psValue->size();
    mutex_.unlock();
    /*
    std::string * poMessage = tData.psValue;
    uint64_t llNowTime = Time::GetSteadyClockMS();
    int iDelayMs = llNowTime > tData.llEnqueueAbsTime ? (int)(llNowTime - tData.llEnqueueAbsTime) : 0;
    BP->GetNetworkBP()->TcpOutQueue(iDelayMs);
    if (iDelayMs > TCP_OUTQUEUE_DROP_TIMEMS)
    {
        //PLErr("drop request because enqueue timeout, nowtime %lu unqueuetime %lu",
                //llNowTime, tData.llEnqueueAbsTime);
        delete poMessage;
        return 0;
    }

    int iBuffLen = poMessage->size();
    int niBuffLen = htonl(iBuffLen + 4);

    int iLen = iBuffLen + 4;
    m_oWriteCacheBuffer.Ready(iLen);
    memcpy(m_oWriteCacheBuffer.GetPtr(), &niBuffLen, 4);
    memcpy(m_oWriteCacheBuffer.GetPtr() + 4, poMessage->c_str(), iBuffLen);

    left_write_len_ = iLen;
    last_write_pos_ = 0;

    delete poMessage;

    //PLImp("write len %d ip %s port %d", iLen, addr_.getHost().c_str(), addr_.getPort());

    int iWriteLen = socket_.send(m_oWriteCacheBuffer.GetPtr(), iLen);
    if (iWriteLen < 0)
    {
        PLErr("fail, write len %d ip %s port %d",
                iWriteLen, addr_.getHost().c_str(), addr_.getPort());
        return -1;
    }

    if (iWriteLen == 0)
    {
        //need wait next write
        AddEvent(EPOLLOUT);

        return 1;
    }

    //PLImp("real write len %d", iWriteLen);

    if (iWriteLen == iLen)
    {
        left_write_len_ = 0;
        last_write_pos_ = 0;
        //write done
    }
    else if (iWriteLen < iLen)
    {
        last_write_pos_ = iWriteLen;
        left_write_len_ = iLen - iWriteLen;

        PLImp("write buflen %d smaller than expectlen %d", iWriteLen, iLen);
    }
    else
    {
        PLErr("write buflen %d large than expectlen %d", iWriteLen, iLen);
    }
    */
    return 0;
}

void MessageEvent::OnError(bool & bNeedDelete) {
    bNeedDelete = false;

    if (type_ == MessageEventType_RECV) {
        bNeedDelete = true;
        return;
    }
    else if (type_ == MessageEventType_SEND) {
        if (IsActive()) {
            AddTimer(200, MessageEventTimerType_Reconnect, reconnect_timeout_id_);
        }
        else {
            mutex_.lock();

            while (!m_oInQueue.empty()) {
                QueueData tData = m_oInQueue.front();
                m_oInQueue.pop();

                delete tData.psValue;
            }

            queue_size_ = 0;

            mutex_.unlock();

            bNeedDelete = true;
            return;
        }
    }
}

void MessageEvent::OnTimeout(const uint32_t iTimerID, const int iType) {
    if (iTimerID != reconnect_timeout_id_) {
        return;
    }

    if (iType == MessageEventTimerType_Reconnect) {
        ReConnect();
    }
}

void MessageEvent::ReConnect() {
    //reset 
    event_ = 0;
    left_write_len_ = 0;
    last_write_pos_ = 0;

    socket_.Reset();
    socket_.SetNonBlocking(true);
    socket_.SetNoDelay(true);
    socket_.Connect(addr_);
    AddEvent(EPOLLOUT);
}

}

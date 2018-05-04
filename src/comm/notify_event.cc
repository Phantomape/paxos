#include "notify_event.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

namespace paxos {

NotifyEvent::NotifyEvent(EventLoop * poEventLoop)
    : Event(poEventLoop) {
    pipe_fd_[0] = -1;
    pipe_fd_[1] = -1;
    host_ = "NotifyEvent";
}

NotifyEvent::~NotifyEvent() {
    for (int i = 0; i < 2; i ++) {
        if (pipe_fd_[i] != -1) {
            close(pipe_fd_[i]);
        }
    }
}

int NotifyEvent::Init() {
    int res = pipe(pipe_fd_);
    if (res != 0) {
        return res;
    }

    fcntl(pipe_fd_[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_fd_[1], F_SETFL, O_NONBLOCK);

    AddEvent(EPOLLIN);
    return 0;
}

int NotifyEvent::GetSocketFd() const {
    return pipe_fd_[0];
}

const std::string & NotifyEvent::GetSocketHost() {
    return host_;
}

void NotifyEvent::SendNotifyEvent() {
    ssize_t write_len = write(pipe_fd_[1], (void *)"a", 1);
    if (write_len != 1) {
        //PLErr("NotifyEvent error, writelen %d", iWriteLen);
    }
}

int NotifyEvent::OnRead() {
    char tmp[2] = {0};
    int read_len = read(pipe_fd_[0], tmp, 1);
    if (read_len < 0) {
        return -1;
    }

    return 0;
}

void NotifyEvent::OnError(bool & need_delete) {
    need_delete = false;
}
    
}



// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#include "Channel.h"

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <queue>

#include "Epoll.h"
#include "EventLoop.h"
#include "Util.h"

Channel::Channel(EventLoop *loop)
    : loop_(loop), fd_(0), events_(0), lastEvents_(0) {}

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), lastEvents_(0) {
        
    }

Channel::~Channel() {
    // loop_->poller_->epoll_del(fd, events_);
    // close(fd_);
}

void Channel::handleRead() {
    if (readHandler_) { readHandler_(); }
}

void Channel::handleWrite() {
    if (writeHandler_) { writeHandler_(); }
}

void Channel::handleConn() {
    if (connHandler_) { connHandler_(); }
}

void Channel::handleError() {
    if (errorHandler_) { errorHandler_(); }
}

void Channel::handleEvents() {
    events_ = 0;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        events_ = 0;
        return;
    }
    if (revents_ & EPOLLERR) {
        handleError();
        events_ = 0;
        return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        handleRead();
    }
    if (revents_ & EPOLLOUT) {
        handleWrite();
    }
    handleConn();
}
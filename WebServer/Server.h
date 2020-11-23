// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _SERVER_H_
#define _SERVER_H_

#include <memory>
#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

class Server {
public:
    Server(EventLoop *loop, int threadNum, int port);
    ~Server() {}
    EventLoop *getLoop() const { return loop_; }
    void start();
    void handNewConn();
    void handThisConn() { loop_->updatePoller(listenChannel_); }

private:
    EventLoop *loop_;
    int threadNum_;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    bool started_;
    int port_;
    int listenFd_; // listenFd_ 在 listenChannel_ 之前声明
    std::shared_ptr<Channel> listenChannel_;
    static const int MAXFDS = 100000;
};

#endif // _SERVER_H_
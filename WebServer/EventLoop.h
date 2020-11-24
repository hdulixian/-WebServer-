// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

#include <functional>
#include <memory>
#include <vector>
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
#include "base/CurrentThread.h"
#include "base/Logging.h"
#include "base/Thread.h"

#include <iostream>

class EventLoop {
 public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor &&cb);
    void queueInLoop(Functor &&cb);
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void assertInLoopThread() { assert(isInLoopThread()); }
    void shutdown(std::shared_ptr<Channel> channel) { shutDownWR(channel->getFd()); }
    void removeFromPoller(std::shared_ptr<Channel> channel) {
        // shutDownWR(channel->getFd());
        poller_->epoll_del(channel);
    }
    void updatePoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller_->epoll_mod(channel, timeout);
    }
    void addToPoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller_->epoll_add(channel, timeout);
    }

 private:
    bool looping_;
    bool quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;
    std::shared_ptr<Epoll> poller_;
    std::vector<Functor> pendingFunctors_;
    mutable MutexLock mutex_;  // 守护 pendingFunctors_
    const pid_t threadId_;
    int wakeupFd_; // wakeupFd_ 在 wakeupChannel_ 之前声明
    std::shared_ptr<Channel> wakeupChannel_;

    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();
};

#endif // _EVENTLOOP_H_
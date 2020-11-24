// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include "Util.h"
#include "base/Logging.h"

__thread EventLoop *t_loopInThisThread = 0;

int createEventfd() {
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        perror("Failed in eventfd");
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
        : looping_(false),
          quit_(false),
          eventHandling_(false),
          callingPendingFunctors_(false),
          poller_(new Epoll()), /* epoll_create1() */
          threadId_(CurrentThread::tid()),
          wakeupFd_(createEventfd()),
          wakeupChannel_(new Channel(this, wakeupFd_)) {
    if (t_loopInThisThread) {
        LOG << "Exception: Another EventLoop " << t_loopInThisThread << " exists in this thread already...";
    } else {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setEvents(EPOLLIN | EPOLLET);
    wakeupChannel_->setReadHandler(std::bind(&EventLoop::handleRead, this));
    poller_->epoll_add(wakeupChannel_, 0);  /* epoll_ctl() */
    // addToPoller(wakeupChannel_, 0);  /* epoll_ctl() */
}

EventLoop::~EventLoop() {
    // wakeupChannel_->disableAll();
    // wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
    if (n != sizeof one) {
        LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleConn() {
    updatePoller(wakeupChannel_, 0);
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
    wakeupChannel_->setEvents(EPOLLIN | EPOLLET);  // ??
}

void EventLoop::runInLoop(Functor &&cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor &&cb) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    if (!isInLoopThread() || callingPendingFunctors_) wakeup();
}

void EventLoop::loop() {
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    quit_ = false;
    LOG << "EventLoop " << this " in thread " << (int)threadId_ << " start looping...";

    std::vector<std::shared_ptr<Channel>> activeChannelList;
    while (!quit_) {
        activeChannelList.clear();
        activeChannelList = poller_->poll();  /* epoll_wait() */
        eventHandling_ = true;
        // mainLoop中只有一个wakeupChannel_和一个acceptChannel_，ioLoop中则有一个wakeupChannel_和多个connChannel_
        // 对应的，mainLoop只负责监听wakeupFd_和listenFd_，而ioLoop则负责监听wakeupFd_以及客户端与多个connFd_之间的通信
        for (auto &activeChannel : activeChannelList) activeChannel->handleEvents(); 
        eventHandling_ = false;
        doPendingFunctors(); // ioLoop对应的pendingFunctors为HttpData::newEvent，它会把connFd_加入epoll监听队列
        poller_->handleExpired();
    }
    looping_ = false;
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    // for (size_t i = 0; i < functors.size(); ++i) functors[i]();
    for (auto &functor : functors) functor();
    callingPendingFunctors_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}
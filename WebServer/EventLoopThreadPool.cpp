// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *mainLoop, int numThreads)
        : mainLoop_(mainLoop), 
          started_(false), 
          numThreads_(numThreads), 
          next_(0) {
    if (numThreads_ <= 0) {
        LOG << "EventLoopThreadPool::numThreads_ = " << numThreads;
        if(numThreads_ < 0) {
            perror("numThreads_ can not be negative...")
            abort();
        }
    }
}

void EventLoopThreadPool::start() {
    mainLoop_->assertInLoopThread();
    started_ = true;
    for (int i = 0; i < numThreads_; i++) {
        std::shared_ptr<EventLoopThread> ioLoopThread(new EventLoopThread());
        ioLoopThreads_.push_back(ioLoopThread);
        ioLoops_.push_back(ioLoopThread->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    mainLoop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = mainLoop_;
    if (!ioLoops_.empty()) {
        loop = ioLoops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}
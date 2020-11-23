// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _EVENTLOOP_THREADPOOL_H_
#define _EVENTLOOP_THREADPOOL_H_

#include <memory>
#include <vector>
#include "EventLoopThread.h"
#include "base/Logging.h"
#include "base/noncopyable.h"


class EventLoopThreadPool : noncopyable {
 public:
    EventLoopThreadPool(EventLoop* mainLoop, int numThreads);
    ~EventLoopThreadPool() { LOG << "~EventLoopThreadPool()"; }
    void start();
    EventLoop *getNextLoop();

 private:
    EventLoop *mainLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread>> ioLoopThreads_;
    std::vector<EventLoop*> ioLoops_;
};

#endif // _EVENTLOOP_THREADPOOL_H_
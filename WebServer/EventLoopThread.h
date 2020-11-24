// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _EVENTLOOP_THREAD_H_
#define _EVENTLOOP_THREAD_H_

#include "EventLoop.h"
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"


class EventLoopThread : noncopyable {
 public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

 private:
    void threadFunc();
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;  // 守护 loop_
    Condition cond_;
};

#endif // _EVENTLOOP_THREAD_H_
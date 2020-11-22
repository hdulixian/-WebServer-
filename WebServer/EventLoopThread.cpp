// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread()
    : loop_(NULL),
        exiting_(false),
        thread_(bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
        mutex_(),
        cond_(mutex_)
{}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != NULL) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL) cond_.wait(); // 一直等到子线程中 loop_ 初始化完成，随后主线程继续执行，子线程则进入事件循环
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    loop.loop();
    loop_ = NULL;
}
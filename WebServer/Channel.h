// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"

class EventLoop;
class HttpData;

class Channel {
public:
    Channel(EventLoop *loop);
    Channel(EventLoop *loop, int fd);
    ~Channel();

    int getFd()  { return fd_; }
    void setFd(int fd)  { fd_ = fd; }

    void setHolder(std::shared_ptr<HttpData> holder) { holder_ = holder; }
    std::shared_ptr<HttpData> getHolder() {
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
    }

    void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }
    void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
    void setWriteHandler(CallBack &&writeHandler) { writeHandler_ = writeHandler; }
    void setErrorHandler(CallBack &&errorHandler) { errorHandler_ = errorHandler; }

    void handleConn();
    void handleRead();
    void handleWrite();
    void handleError();

    void handleEvents();

    void setRevents(__uint32_t ev) { revents_ = ev; }
    void setEvents(__uint32_t ev) { events_ = ev; }
    __uint32_t &getEvents() { return events_; }
    __uint32_t getLastEvents() { return lastEvents_; }

    bool EqualAndUpdateLastEvents() {
        bool ret = (lastEvents_ == events_);
        lastEvents_ = events_;
        return ret;
    }

private:
    typedef std::function<void()> CallBack;
    EventLoop *loop_;
    int fd_;
    __uint32_t events_;
    __uint32_t revents_;
    __uint32_t lastEvents_;
    std::weak_ptr<HttpData> holder_; // 方便找到上层持有该Channel的对象

private:
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;
};

#endif // _CHANNEL_H_
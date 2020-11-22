// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _COUNTDOWNLATCH_H_
#define _COUNTDOWNLATCH_H_

#include "Condition.h"
#include "MutexLock.h"
#include "noncopyable.h"
#include <mutex>

// CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后
// 外层的start才返回
class CountDownLatch : noncopyable {
 public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();

 private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};

#endif // _COUNTDOWNLATCH_H_
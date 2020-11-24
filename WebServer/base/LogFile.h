// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _LOG_FILE_H_
#define _LOG_FILE_H_

#include <memory>
#include <string>
#include "FileUtil.h"
#include "MutexLock.h"
#include "noncopyable.h"


// TODO 提供自动归档功能
class LogFile : noncopyable {
 public:
    // 每被append N次就flush一下，会往文件写，只不过，文件也是带缓冲区的
    LogFile(const string& basename, int flushEveryN = 1024);
    ~LogFile();

    void append(const char *logline, int len);
    void flush();
    bool rollFile();

 private:
    void append_unlocked(const char *logline, int len);

    const string basename_;
    const int flushEveryN_;

    int count_;
    std::unique_ptr<MutexLock> mutex_;
    std::unique_ptr<AppendFile> file_;
};

#endif // _LOG_FILE_H_
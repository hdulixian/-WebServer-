// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _FILE_UTIL_H_
#define _FILE_UTIL_H_

#include <string>
#include "noncopyable.h"


class AppendFile : noncopyable {
 public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    // append 会向文件写
    void append(const char *logline, const size_t len);
    void flush();

 private:
    size_t write(const char *logline, size_t len);
    FILE *fp_;
    char buffer_[64 * 1024];
};

#endif // _FILE_UTIL_H_
// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <pthread.h>
#include <stdio.h>
#include <string>
#include "LogStream.h"

class AsyncLogging;

class Logger {
public:
    Logger(const char *fileName, int line);
    ~Logger();
    LogStream &stream() { return impl_.stream_; }

    static void setLogFileName(string fileName) { logFileName_ = fileName; }
    static string getLogFileName() { return logFileName_; }

private:
    class Impl {
     public:
        Impl(const char *fileName, int line);
        void formatTime();

        LogStream stream_;
        int line_;
        string basename_;
    };
    
    Impl impl_;
    static string logFileName_;
};

#define LOG Logger(__FILE__, __LINE__).stream()

#endif // _LOGGING_H_
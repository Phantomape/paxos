#pragma once

#include <stdarg.h>
#include <functional>

namespace paxos {

enum class LogLevel {
    LogLevel_None = 0,
    LogLevel_Error = 1,
    LogLevel_Warning = 2,
    LogLevel_Info = 3,
    LogLevel_Verbose = 4,
};

typedef std::function< void(const int, const char *, va_list) > LogFunc;
    
}

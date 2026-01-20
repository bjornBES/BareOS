#include <stdint.h>

#ifndef DEBUGLEVELDEF
#define DEBUGLEVELDEF 1
typedef enum {
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 2,
    LVL_ERROR = 3,
    LVL_CRITICAL = 4
} DebugLevel;
#endif

void logf(const char* module, DebugLevel level, const char* fmt, ...);
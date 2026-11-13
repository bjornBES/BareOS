/*
 * File: kerrno.h
 * File Created: 15 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "debug/debug.h"
#include <kerrno_uapi.h>

// Design from https://github.com/haiku/haiku/blob/master/headers/os/support/Errors.h

#define RETURN_SUCCESSES()       \
    {                            \
        return KERRNO_SUCCESSES; \
    }

#define KERRNO_NO_RETURN(number, ...)                                      \
    {                                                                      \
        trace_err(MODULE, "%s:%u", __FILE__, __LINE__);                    \
        trace_err(MODULE, "func %s outputting %s", __FUNCTION__, #number); \
        trace_info(MODULE, __VA_ARGS__);                                   \
    }

#define LOG_RETURN(value)                                                  \
    {                                                                      \
        trace_info(MODULE, "%s:%u", __FILE__, __LINE__);                    \
        trace_info(MODULE, "func %s returning 0x%llx", __FUNCTION__, value); \
        return value;                                                      \
    }

#define KERRNO_RETURN(number, ...)             \
    {                                          \
        KERRNO_NO_RETURN(#number, __VA_ARGS__) \
        return number;                         \
    }

#define KERRNO_RETURN_TYPE(number, type, ...)  \
    {                                          \
        KERRNO_NO_RETURN(#number, __VA_ARGS__) \
        return (type)number;                   \
    }

#if DEBUG == 1
#define KERRNO_RETURN_DEBUG(number, type, ...)                                                       \
    {                                                                                                \
        trace_info(NO_MODULE, "Debugging \"%s\" becurse it returned \"%s\"", __FUNCTION__, #number); \
        KERRNO_RETURN_TYPE(number, type, __VA_ARGS__);                                               \
    }
#define KERRNO_CHECK_RETURN(number)                                                                \
    {                                                                                              \
        if (number != KERRNO_SUCCESSES)                                                            \
        {                                                                                          \
            trace_warn(MODULE, "In %s at %s:%u got %u", __FUNCTION__, __FILE__, __LINE__, number); \
        }                                                                                          \
    }
#else
#error "DEBUG is disabled"
#define KERRNO_RETURN_DEBUG(number, type, ...)         \
    {                                                  \
        KERRNO_RETURN_TYPE(number, type, __VA_ARGS__); \
    }
#define KERRNO_CHECK_RETURN(number)
#endif

#if DEBUG == 1
#define KERRNO_CHECK_DEBUG(status)                                                                                                                          \
    {                                                                                                                                                       \
        if (status == KERRNO_DEBUG)                                                                                                                         \
        {                                                                                                                                                   \
            trace_info(NO_MODULE, "Debug:\nCalled in \"%s\" on around line %u\n%s:%s in \"%s\"", __FUNCTION__, __LINE__, __FILE__, __LINE__, __FUNCTION__); \
        }                                                                                                                                                   \
    }
#else
#define KERRNO_CHECK_DEBUG(status) \
    {                              \
    }
#endif


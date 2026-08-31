/*
 * File: errno.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <base_errno.h>
#include "debug/debug.h"

#define SET_ERROR(number) errno = number;

#define ERRNO_RETURN(number, ...)                                        \
    {                                                                    \
        SET_ERROR(number)                                                \
        log_err(MODULE, "func %s outputting %s", __FUNCTION__, #number); \
        logfl(MODULE, LVL_INFO, __VA_ARGS__);                            \
        return number;                                                   \
    }

#define ERRNO_NO_RETURN(number, ...)                                     \
    {                                                                    \
        SET_ERROR(number)                                                \
        log_err(MODULE, "func %s outputting %s", __FUNCTION__, #number); \
        logfl(MODULE, LVL_INFO, __VA_ARGS__);                            \
    }

extern int errno;

/*
 * File: debug.h
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "stdio.h"

#define ENTER_FUNC(args, ...)                       \
    {                                               \
        printf("enter function %s(", __FUNCTION__); \
        printf(args, __VA_ARGS__);                  \
        printf(")\n");                              \
    }

#define EXIT_FUNC(ret)                                          \
    {                                                           \
        printf("exit function %s() = %i\n", __FUNCTION__, ret); \
    }


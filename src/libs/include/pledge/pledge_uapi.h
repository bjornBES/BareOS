/*
 * File: pledge_uapi.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <binary.h>

typedef enum pledge_flags
{
    PLEDGE_STDIO = BIT(0),  // read/write to existing fds
    PLEDGE_RPATH = BIT(1),  // read files
    PLEDGE_WPATH = BIT(2),  // write files
    PLEDGE_PROC = BIT(3),   // fork/exec
    PLEDGE_MALLOC = BIT(4), // memory allocation
    PLEDGE_EXEC = BIT(5),   // execve
} pledge_flags_t;

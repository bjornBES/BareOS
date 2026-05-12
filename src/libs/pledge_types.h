/*
 * File: pledge_types.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

typedef enum {
    PLEDGE_STDIO    = (1 << 0),  // read/write to existing fds
    PLEDGE_RPATH    = (1 << 1),  // read files
    PLEDGE_WPATH    = (1 << 2),  // write files
    PLEDGE_PROC     = (1 << 3),  // fork/exec
    PLEDGE_MALLOC   = (1 << 4),  // memory allocation
    PLEDGE_EXEC     = (1 << 5),  // execve
} pledge_flags_t;
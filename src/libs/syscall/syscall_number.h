/*
 * File: syscall_number.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define X(nr, name, sysname, sysnameupper, ...) SYS_##sysnameupper = nr,

typedef enum
{
#include "syscall_table.tbl"
    SYSCALL_COUNT
} syscall_nr_t;

#undef X

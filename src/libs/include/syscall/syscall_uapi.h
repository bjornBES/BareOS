/*
 * File: syscall_uapi.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define X(nr, name, sysname, sysnameupper, ret, pledge, abi, _sys, version, ...) SYS_##sysnameupper = nr,

typedef enum
{
#include "syscall_table_bareos.tbl"
    SYSCALL_COUNT
} syscall_nr_t;

#undef X

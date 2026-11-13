/*
 * File: syscallt.h
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/syscall_archt.h"

#include "pledge/pledge_uapi.h"

#include "loader/loadert.h"

typedef status_t (*syscall_handler)(syscall_info_t *info, intptr_t *out_ret);

typedef struct syscall_function_info
{
    uint32_t number;
    syscall_handler handler;
    bool call_dose_return;
    pledge_flags_t pledge;
    loader_abi_t abi;
} syscall_function_info_t;

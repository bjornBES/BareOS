/*
 * File: syscall_archt.h
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "type_arch.h"

#include <types.h>

typedef struct syscall_info
{
    intr_frame_t *regs;
    reg_t sys_number;
    uint32_t number_of_args;
    reg_t args[6];
    uint32_t extra_on_stack;
} syscall_info_t;
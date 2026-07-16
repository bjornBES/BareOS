/*
 * File: Syscall.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "ctx.h"

typedef struct
{
    intr_frame_t *regs;
    reg_t sys_number;
    uint32_t number_of_args;
    reg_t arg1;
    reg_t arg2;
    reg_t arg3;
    reg_t arg4;
    reg_t arg5;
    reg_t arg6;
    uint32_t extra_on_stack;
} syscall_info;

void arch_syscall_init();

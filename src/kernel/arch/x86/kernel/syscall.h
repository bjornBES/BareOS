/*
 * File: syscall.h
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "registers.h"

typedef struct
{
    registers *regs;
    reg_t sys_number;
    reg_t arg1;
    reg_t arg2;
    reg_t arg3;
    reg_t arg4;
    reg_t arg5;
    reg_t arg6;
} syscall_info;

void arch_syscall_init();

/*
 * File: exception.h
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/registers.h"

#define EXC_PAGE 14
#define EXC_SYSCALL 0x80

typedef void (*kernel_handler)(void *info);
typedef void (*arch_handler)(registers *regs, kernel_handler kernel_hand);

void exception_register_arch_handler(int interrupt, arch_handler handler);
void exception_register_kernel_handler(int interrupt, kernel_handler handler);
void exception_format_registers(registers *regs, char *buf);
void exception_init();
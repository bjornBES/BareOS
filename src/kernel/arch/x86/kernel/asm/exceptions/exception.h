/*
 * File: exception.h
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "isr.h"

#define EXC_DB      1  // Debug
#define EXC_BP      3  // Breakpoint
#define EXC_UD      6  // Invalid Opcode
#define EXC_DF      8  // Double Fault
#define EXC_GP      13 // General Protection Fault
#define EXC_PF      14 // Page Fault
#define EXC_SYSCALL 0x80

typedef int (*arch_handler)(intr_frame_t *regs);

void exception_register_arch_handler(int interrupt, arch_handler handler);
void exception_init();

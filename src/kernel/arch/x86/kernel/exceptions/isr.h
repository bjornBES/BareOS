/*
 * File: isr.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include "kernel/registers.h"
#include "exception.h"

void isr_show_stack_trace(uint32_t max_frames, registers *regs);
void x86_isr_initialize();
void x86_isr_register_handler(int interrupt);
void x86_isr_format_registers(registers *regs, char *buf, char* postfix, char* prefix);
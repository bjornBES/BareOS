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

typedef struct
{
	uint64_t ds;
	uint64_t edi;
	uint64_t esi;
	uint64_t ebp;
	uint64_t _esp;
	uint64_t ebx;
	uint64_t edx;
	uint64_t ecx;
	uint64_t eax;
	uint64_t interrupt, error;
	uint64_t eip, cs, eflags,esp, ss;
} __attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers *regs);

void x86_ISRInitialize();
void x86_isr_register_handler(int interrupt, ISRHandler handler);

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
	uint32_t ds;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t _esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t interrupt, error;
	uint32_t eip, cs, eflags,esp, ss;
} __attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers *regs);

void i686_ISRInitialize();
void i686_isr_register_handler(int interrupt, ISRHandler handler);

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

#ifdef __x86_64__
typedef uint64_t reg_t;
#else
typedef uint32_t reg_t;
#endif

typedef struct
{
	reg_t ds;
#ifdef __x86_64__
	reg_t r15;
	reg_t r14;
	reg_t r13;
	reg_t r12;
	reg_t r11;
	reg_t r10;
	reg_t r9;
	reg_t r8;
#endif
	reg_t di;
	reg_t si;
	reg_t bp;
	reg_t bx;
	reg_t dx;
	reg_t cx;
	reg_t ax;

	reg_t interrupt;
	reg_t error;

	reg_t pc;
	reg_t cs;
	reg_t flags;
	reg_t sp;
	reg_t ss;
} Registers;

typedef void (*ISRHandler)(Registers *regs);

void ISR_show_stack_trace(uint32_t max_frames, Registers *regs);
void x86_ISRInitialize();
void x86_isr_register_handler(int interrupt, ISRHandler handler);
void x86_ISRFormatRegisters(Registers *regs, char *buf, char* postfix, char* prefix);
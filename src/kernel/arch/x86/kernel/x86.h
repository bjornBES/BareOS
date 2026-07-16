/*
 * File: i686.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>
#include <stdint.h>
#include "stdio.h"
#include "debug/debug.h"

#define X86_EFLAGS_CF_BIT		0 /* Carry Flag */
#define X86_EFLAGS_CF			1 << X86_EFLAGS_CF_BIT
#define X86_EFLAGS_FIXED_BIT	1 /* Bit 1 - always on */
#define X86_EFLAGS_FIXED		1 << X86_EFLAGS_FIXED_BIT
#define X86_EFLAGS_PF_BIT		2 /* Parity Flag */
#define X86_EFLAGS_PF			1 << X86_EFLAGS_PF_BIT
#define X86_EFLAGS_AF_BIT		4 /* Auxiliary carry Flag */
#define X86_EFLAGS_AF			1 << X86_EFLAGS_AF_BIT
#define X86_EFLAGS_ZF_BIT		6 /* Zero Flag */
#define X86_EFLAGS_ZF			1 << X86_EFLAGS_ZF_BIT
#define X86_EFLAGS_SF_BIT		7 /* Sign Flag */
#define X86_EFLAGS_SF			1 << X86_EFLAGS_SF_BIT
#define X86_EFLAGS_TF_BIT		8 /* Trap Flag */
#define X86_EFLAGS_TF			1 << X86_EFLAGS_TF_BIT
#define X86_EFLAGS_IF_BIT		9 /* Interrupt Flag */
#define X86_EFLAGS_IF			(1 << X86_EFLAGS_IF_BIT)
#define X86_EFLAGS_DF_BIT		10 /* Direction Flag */
#define X86_EFLAGS_DF			1 << X86_EFLAGS_DF_BIT
#define X86_EFLAGS_OF_BIT		11 /* Overflow Flag */
#define X86_EFLAGS_OF			1 << X86_EFLAGS_OF_BIT
#define X86_EFLAGS_IOPL_BIT		12 /* I/O Privilege Level (2 bits) */
#define X86_EFLAGS_IOPL			3 << X86_EFLAGS_IOPL_BIT
#define X86_EFLAGS_NT_BIT		14 /* Nested Task */
#define X86_EFLAGS_NT			1 << X86_EFLAGS_NT_BIT
#define X86_EFLAGS_RF_BIT		16 /* Resume Flag */
#define X86_EFLAGS_RF			1 << X86_EFLAGS_RF_BIT
#define X86_EFLAGS_VM_BIT		17 /* Virtual Mode */
#define X86_EFLAGS_VM			1 << X86_EFLAGS_VM_BIT
#define X86_EFLAGS_AC_BIT		18 /* Alignment Check/Access Control */
#define X86_EFLAGS_AC			1 << X86_EFLAGS_AC_BIT
#define X86_EFLAGS_VIF_BIT		19 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIF			1 << X86_EFLAGS_VIF_BIT
#define X86_EFLAGS_VIP_BIT		20 /* Virtual Interrupt Pending */
#define X86_EFLAGS_VIP			1 << X86_EFLAGS_VIP_BIT
#define X86_EFLAGS_ID_BIT		21 /* CPUID detection */
#define X86_EFLAGS_ID			1 << X86_EFLAGS_ID_BIT

void arch_breakpoint();
void ARCH_CALL arch_panic();

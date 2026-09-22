/*
 * File: cpu_flags.h
 * File Created: 21 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <binary.h>

#define X86_EFLAGS_CF_BIT		0 /* Carry Flag */
#define X86_EFLAGS_CF			BIT(X86_EFLAGS_CF_BIT)
#define X86_EFLAGS_FIXED_BIT	1 /* Bit 1 - always on */
#define X86_EFLAGS_FIXED		BIT(X86_EFLAGS_FIXED_BIT)
#define X86_EFLAGS_PF_BIT		2 /* Parity Flag */
#define X86_EFLAGS_PF			BIT(X86_EFLAGS_PF_BIT)
#define X86_EFLAGS_AF_BIT		4 /* Auxiliary carry Flag */
#define X86_EFLAGS_AF			BIT(X86_EFLAGS_AF_BIT)
#define X86_EFLAGS_ZF_BIT		6 /* Zero Flag */
#define X86_EFLAGS_ZF			BIT(X86_EFLAGS_ZF_BIT)
#define X86_EFLAGS_SF_BIT		7 /* Sign Flag */
#define X86_EFLAGS_SF			BIT(X86_EFLAGS_SF_BIT)
#define X86_EFLAGS_TF_BIT		8 /* Trap Flag */
#define X86_EFLAGS_TF			BIT(X86_EFLAGS_TF_BIT)
#define X86_EFLAGS_IF_BIT		9 /* Interrupt Flag */
#define X86_EFLAGS_IF			BIT(X86_EFLAGS_IF_BIT)
#define X86_EFLAGS_DF_BIT		10 /* Direction Flag */
#define X86_EFLAGS_DF			BIT(X86_EFLAGS_DF_BIT)
#define X86_EFLAGS_OF_BIT		11 /* Overflow Flag */
#define X86_EFLAGS_OF			BIT(X86_EFLAGS_OF_BIT)
#define X86_EFLAGS_IOPL_BIT		12 /* I/O Privilege Level (2 bits) */
#define X86_EFLAGS_IOPL			3 << X86_EFLAGS_IOPL_BIT
#define X86_EFLAGS_NT_BIT		14 /* Nested Task */
#define X86_EFLAGS_NT			BIT(X86_EFLAGS_NT_BIT)
#define X86_EFLAGS_RF_BIT		16 /* Resume Flag */
#define X86_EFLAGS_RF			BIT(X86_EFLAGS_RF_BIT)
#define X86_EFLAGS_VM_BIT		17 /* Virtual Mode */
#define X86_EFLAGS_VM			BIT(X86_EFLAGS_VM_BIT)
#define X86_EFLAGS_AC_BIT		18 /* Alignment Check/Access Control */
#define X86_EFLAGS_AC			BIT(X86_EFLAGS_AC_BIT)
#define X86_EFLAGS_VIF_BIT		19 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIF			BIT(X86_EFLAGS_VIF_BIT)
#define X86_EFLAGS_VIP_BIT		20 /* Virtual Interrupt Pending */
#define X86_EFLAGS_VIP			BIT(X86_EFLAGS_VIP_BIT)
#define X86_EFLAGS_ID_BIT		21 /* CPUID detection */
#define X86_EFLAGS_ID			BIT(X86_EFLAGS_ID_BIT)

#pragma once

#include <core/Defs.h>
#include <stdint.h>

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
#define X86_EFLAGS_IF			1 << X86_EFLAGS_IF_BIT
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

uint16_t ds(void);

void set_fs(uint16_t seg);
uint16_t fs(void);

void set_gs(uint16_t seg);
uint16_t gs(void);


typedef struct _biosregs {
    union {
        struct {
            uint32_t edi;
			uint32_t esi;
			uint32_t ebp;
			uint32_t _esp;
			uint32_t ebx;
			uint32_t edx;
			uint32_t ecx;
			uint32_t eax;
			uint32_t _fsgs;
			uint32_t _dses;
			uint32_t eflags;
		};
		struct {
            uint16_t di, hdi;
			uint16_t si, hsi;
			uint16_t bp, hbp;
			uint16_t _sp, _hsp;
			uint16_t bx, hbx;
			uint16_t dx, hdx;
			uint16_t cx, hcx;
			uint16_t ax, hax;
			uint16_t gs, fs;
			uint16_t es, ds;
			uint16_t flags, hflags;
		};
		struct {
            uint8_t dil, dih, edi2, edi3;
			uint8_t sil, sih, esi2, esi3;
			uint8_t bpl, bph, ebp2, ebp3;
			uint8_t _spl, _sph, _esp2, _esp3;
			uint8_t bl, bh, ebx2, ebx3;
			uint8_t dl, dh, edx2, edx3;
			uint8_t cl, ch, ecx2, ecx3;
			uint8_t al, ah, eax2, eax3;
		};
	};
} biosregs;

void initregs(biosregs *reg);
extern ASMCALL void intcall(uint8_t int_no, biosregs *ireg, biosregs *oreg);
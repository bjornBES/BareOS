/*
 * File: MSR.h
 * File Created: 06 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <util/binary.h>
#include <defs.h>

#define MSR_IA32_APIC_BASE        0x0000001B
#define MSR_IA32_APIC_BASE_BSP    8
#define MSR_IA32_APIC_BASE_ENABLE 11
#define MSR_IA32_APIC_BASE_BASE   (0xFFFFF << 12)

#define MSR_IA32_SYSENTER_CS      0x00000174
#define MSR_IA32_SYSENTER_ESP     0x00000175
#define MSR_IA32_SYSENTER_EIP     0x00000176

/* x86-64 specific MSRs */
#define MSR_EFER           0xC0000080
#define MSR_EFER_SCE       0          /* SYSCALL/SYSRET */
#define MSR_EFER_LME       8          /* Long mode enable */
#define MSR_EFER_LMA       10         /* Long mode active (read-only) */
#define MSR_EFER_NX        11         /* No execute enable */
#define MSR_EFER_SVME      12         /* Enable virtualization */
#define MSR_EFER_LMSLE     13         /* Long Mode Segment Limit Enable */
#define MSR_EFER_FFXSR     14         /* Enable Fast FXSAVE/FXRSTOR */
#define MSR_EFER_TCE       15         /* Enable Translation Cache Extensions */
#define MSR_EFER_AUTOIBRS  21         /* Enable Automatic IBRS */

#define MSR_STAR           0xC0000081 /* legacy mode SYSCALL target */
#define MSR_LSTAR          0xC0000082 /* long mode SYSCALL target */
#define MSR_CSTAR          0xC0000083 /* compat mode SYSCALL target */
#define MSR_SYSCALL_MASK   0xC0000084 /* EFLAGS mask for syscall */
#define MSR_FS_BASE        0xC0000100 /* 64bit FS base */
#define MSR_GS_BASE        0xC0000101 /* 64bit GS base */
#define MSR_KERNEL_GS_BASE 0xC0000102 /* 64bit kernel GS base */

int msr_has_msr();

#define wrmsr(msr, value) msr_set_64(msr, value)

static inline void msr_get_32(uint32_t msr, uint32_t *low, uint32_t *high)
{
    inline_asm("rdmsr" : "=a"(*low), "=d"(*high) : "c"(msr));
}

static inline void msr_set_32(uint32_t msr, uint32_t low, uint32_t high)
{
    inline_asm("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

static inline uint64_t msr_get_64(uint32_t msr)
{
    uint32_t low;
    uint32_t high;
    inline_asm("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void msr_set_64(uint32_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = (value >> 32);
    inline_asm("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

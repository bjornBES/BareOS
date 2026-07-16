/*
 * File: processor_ctl.h
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "processor_flags.h"

static inline void ctl_cr0_add(uint64_t flags)
{
    uint64_t cr0;
    __asm__ volatile("mov %0, cr0" : "=r"(cr0) : : "rax");
    cr0 |= flags;
    __asm__ volatile("mov cr0, %0" : : "r"(cr0) : "rax");
}
static inline void ctl_cr4_add(uint64_t flags)
{
    uint64_t cr4;
    __asm__ volatile("mov %0, cr4" : "=r"(cr4) : : "rax");
    cr4 |= flags;
    __asm__ volatile("mov cr4, %0" : : "r"(cr4) : "rax");
}

static inline void ctl_cr0_remove(uint64_t flags)
{
    uint64_t cr0;
    __asm__ volatile("mov %0, cr0" : "=r"(cr0) : : "rax");
    cr0 &= ~flags;
    __asm__ volatile("mov cr0, %0" : : "r"(cr0) : "rax");
}
static inline void ctl_cr4_remove(uint64_t flags)
{
    uint64_t cr4;
    __asm__ volatile("mov %0, cr4" : "=r"(cr4) : : "rax");
    cr4 &= ~flags;
    __asm__ volatile("mov cr4, %0" : : "r"(cr4) : "rax");
}

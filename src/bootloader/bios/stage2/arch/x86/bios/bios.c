/*
 * File: bios.c
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "bios.h"

#include <defs.h>

#include "memory.h"
#include "stdio.h"
#include "debug/debug.h"

extern void bios_intcall(uint8_t int_no, bios_regs_t *ireg, bios_regs_t *oreg);

SECTION("arch_text") void bios_arch_init_regs(bios_regs_t *reg)
{
    memset(reg, 0, sizeof(bios_regs_t));
    reg->flags |= 1;
    reg->ds = bios_get_ds();
    reg->es = bios_get_ds();
    reg->fs = bios_get_fs();
    reg->gs = bios_get_gs();
}

SECTION("arch_text") void bios_arch_dump_frame(bios_regs_t *frame)
{
    printf("ax: %x, bx: %x, cx: %x, dx: %x\n", frame->ax, frame->bx, frame->cx, frame->dx);
    printf("si: %x, di: %x\n", frame->si, frame->di);
    printf("flags: %x\n", frame->flags);
    printf("ds: %x, es: %x, fs: %x, gs: %x\n", frame->ds, frame->es, frame->fs, frame->gs);
}

void bios_arch_intcall(uint8_t int_no, bios_regs_t *ireg, bios_regs_t *oreg)
{
    // ENTER_FUNC("0x%x, 0x%p, 0x%p", int_no, ireg, oreg);

/*     uint32_t stack_pointer;
    uint32_t base_pointer;
    __asm__("mov %0, esp" : "=r"(stack_pointer));
    __asm__("mov %0, ebp" : "=r"(base_pointer));
    printf("*base_pointer = 0x%lx\n", *((uint32_t *)base_pointer));
    printf("*base_pointer + 1 = 0x%lx\n", *((uint32_t *)base_pointer + 1));
    base_pointer = *((uint32_t *)base_pointer);
    printf("*base_pointer = 0x%lx\n", *((uint32_t *)base_pointer));
    printf("*base_pointer + 1 = 0x%lx\n", *((uint32_t *)base_pointer + 1));
    base_pointer = *((uint32_t *)base_pointer);
    printf("*base_pointer = 0x%lx\n", *((uint32_t *)base_pointer));
    printf("*base_pointer + 1 = 0x%lx\n", *((uint32_t *)base_pointer + 1));
    base_pointer = *((uint32_t *)base_pointer);
    printf("*base_pointer = 0x%lx\n", *((uint32_t *)base_pointer));
    printf("*base_pointer + 1 = 0x%lx\n", *((uint32_t *)base_pointer + 1)); */

    bios_intcall(int_no, ireg, oreg);
}

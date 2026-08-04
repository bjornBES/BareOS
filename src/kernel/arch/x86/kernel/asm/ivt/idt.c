/*
 * File: idt.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "idt.h"
#include <util/binary.h>
#include <defs.h>
#include "debug/debug.h"

#define MODULE "x86-IVT"

#ifdef __i686__
void ASMCALL32 idt_load_32(idtr_t *descriptor);
#else
void idt_load_64(idtr_t *descriptor);
#endif

idt_entry kernel_idt_table[256];
idtr_t kernel_idtr = {sizeof(kernel_idt_table) - 1, kernel_idt_table};

#ifdef __i686__
void x86_idt_set_gate(uint32_t interrupt, void *base, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
#else
void x86_idt_set_gate(uint32_t interrupt, void *base, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
#endif
{
    idt_set_gate(interrupt, base, segment_descriptor, flags, ist);
}

#ifdef __i686__
void idt_set_gate(uint32_t interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t res)
#else
void idt_set_gate(uint32_t interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
#endif
{
    uint32_64 base = (uint32_64)callback;
    kernel_idt_table[interrupt].base_1 = ((uint32_t)base) & 0xFFFF;
    kernel_idt_table[interrupt].segment_selector = segment_descriptor;
    kernel_idt_table[interrupt].reserved1 = 0;
    kernel_idt_table[interrupt].flags = flags;
    kernel_idt_table[interrupt].base_2 = ((uint32_t)base >> 16) & 0xFFFF;
#ifdef __x86_64__
    kernel_idt_table[interrupt].ist = ist;
    kernel_idt_table[interrupt].base_3 = (base >> 32);
#endif
}

void x86_idt_disable_gate(uint32_t interrupt)
{
    FLAG_UNSET(kernel_idt_table[interrupt].flags, IDT_FLAG_PRESENT);
}

void x86_idt_enable_gate(uint32_t interrupt)
{
    FLAG_SET(kernel_idt_table[interrupt].flags, IDT_FLAG_PRESENT);
}

void x86_idt_load()
{
    kernel_idtr.size = sizeof(kernel_idt_table) - 1;
    kernel_idtr.ptr = kernel_idt_table;

    // log_debug(MODULE, "loading IDT");
#ifdef __x86_64__
    inline_asm(
        "cli\n\t"
        "lidt %0"
        : : "m"(kernel_idtr) : "memory");
    // idt_load_64(&kernel_idtr);
#else
    IDT_Load_32(&kernel_idtr);
#endif
    log_debug(MODULE, "loaded IDT");
}

void ivt_arch_init()
{
    x86_idt_load();
    log_debug(MODULE, "IDT is loaded");
}

void ivt_dump_frame(intr_frame_t *frame)
{
    // ENTER_FUNC(MODULE, "%p", frame);
    log_debug(NO_MODULE, "\t{ frame @ %p }", frame);
    log_debug(NO_MODULE, "\t{ ax = 0x%lx, bx = 0x%lx, cx = 0x%lx, dx = 0x%lx }", frame->ax, frame->bx, frame->cx, frame->dx);
    log_debug(NO_MODULE, "\t{ di = 0x%lx, si = 0x%lx, r8 = 0x%lx, r9 = 0x%lx }", frame->di, frame->si, frame->r8, frame->r9);
    log_debug(NO_MODULE, "\t{ r10 = 0x%lx, r11 = 0x%lx, r12 = 0x%lx, r13 = 0x%lx }", frame->r10, frame->r11, frame->r12, frame->r13);
    log_debug(NO_MODULE, "\t{ r14 = 0x%lx, r15 = 0x%lx, bp = 0x%lx, flags = 0x%lx }", frame->r14, frame->r15, frame->bp, frame->flags);
    log_debug(NO_MODULE, "\t{ pc = 0x%x:%p, sp = 0x%x:%p }", frame->cs, frame->pc, frame->ss, frame->sp);
}

// debug function
#ifdef DEBUG

void x86_idt_dump_selector(uint16_t sel)
{
    idtr_t idtr;
    __asm__("sidt %0" : "=m"(idtr));
    idt_entry *idt = (idt_entry *)(idtr.ptr);
    uint16_t index = sel >> 3;
    idt_entry e = idt[index];
#ifdef __i686__
    uint32_t offset = ((uint32_t)e.base_1 << 16) | e.base_2;
#else
    uint64_t offset = ((uint64_t)e.base_1 << 48) | ((uint64_t)e.base_2 << 32) | e.base_3;
#endif
    fprintf(stddebug, "IDT[%02x]: offset=0x%08X selector=0x%04X type_attr=0x%02X",
            index, offset, e.segment_selector, e.flags);
}

void x86_idt_dump_selector_index(uint16_t index)
{
    idtr_t idtr;
    __asm__("sidt %0" : "=m"(idtr));
    idt_entry *idt = (idt_entry *)(idtr.ptr);
    idt_entry e = idt[index];
#ifdef __i686__
    uint32_t offset = ((uint32_t)e.base_1 << 16) | e.base_2;
#else
    uint64_t offset = ((uint64_t)e.base_1 << 48) | ((uint64_t)e.base_2 << 32) | e.base_3;
#endif
    fprintf(stddebug, "IDT[%02x]: offset=0x%08X selector=0x%04X type_attr=0x%02X",
            index, offset, e.segment_selector, e.flags);
}

#endif

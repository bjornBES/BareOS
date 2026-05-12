/*
 * File: idt.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "idt.h"
#include <util/binary.h>
#include "debug/debug.h"

#define MODULE "IDT"

#ifdef __i686__
void ASMCALL32 idt_load_32(idtr_t *descriptor);
#else
void idt_load_64(idtr_t *descriptor);
#endif

idt_entry kernel_idt_table[256];
idtr_t kernel_idtr = {sizeof(kernel_idt_table) - 1, kernel_idt_table};

#ifdef __i686__
void x86_idt_set_gate(int interrupt, void *base, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
#else
void x86_idt_set_gate(int interrupt, void *base, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
#endif
{
    idt_set_gate(kernel_idt_table, interrupt, base, segment_descriptor, flags, ist);
}

#ifdef __i686__
void idt_set_gate(idt_entry table[256], int interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t res)
#else
void idt_set_gate(idt_entry table[256], int interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
#endif
{
    uint32_64 base = (uint32_64)callback;
    table[interrupt].base_1 = ((uint32_t)base) & 0xFFFF;
    table[interrupt].segment_selector = segment_descriptor;
    table[interrupt].reserved1 = 0;
    table[interrupt].flags = flags;
    table[interrupt].base_2 = ((uint32_t)base >> 16) & 0xFFFF;
#ifdef __x86_64__
    table[interrupt].ist = ist;
    table[interrupt].base_3 = (base >> 32);
#endif
}

void x86_idt_disable_gate(idt_entry table[256],int interrupt)
{
    FLAG_UNSET(table[interrupt].flags, IDT_FLAG_PRESENT);
}

void x86_idt_enable_gate(idt_entry table[256],int interrupt)
{
    FLAG_SET(table[interrupt].flags, IDT_FLAG_PRESENT);
}

void x86_idt_load(idtr_t idtr, idt_entry table[256])
{
    idtr.size = sizeof(kernel_idt_table) - 1;
    idtr.ptr = kernel_idt_table;
    
    log_debug(MODULE, "loading IDT");
#ifdef __x86_64__
    idt_load_64(&idtr);
#else
    IDT_Load_32(&idtr);
#endif
log_debug(MODULE, "loaded IDT");
}

void x86_idt_initialize()
{
}

// debug function
#ifdef DEBUG

void x86_idt_dump_selector(uint16_t sel)
{
    idtr_t idtr;
    __asm__("sidt %0" : "=m"(idtr));
    idt_entry *idt = (idt_entry *)(idtr.ptr);
    idt_entry e = idt[sel];
#ifdef __i686__
    uint32_t offset = ((uint32_t)e.base_1 << 16) | e.base_2;
#else
    uint64_t offset = ((uint64_t)e.base_1 << 48) | ((uint64_t)e.base_2 << 32) | e.base_3;
#endif
    fprintf(stddebug, "IDT[%02x]: offset=0x%08X selector=0x%04X type_attr=0x%02X",
            sel, offset, e.segment_selector, e.flags);
}

#endif
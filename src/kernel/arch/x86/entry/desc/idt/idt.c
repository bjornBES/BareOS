/*
 * File: idt.c
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "idt.h"

#include "kernel/isr/isr.h"

#include "debug/debug.h"

#include <binary.h>
#include <defs.h>

#define MODULE "x86-idt"

idt_entry_t kernel_idt_table[256];
idtr_t kernel_idtr = {sizeof(kernel_idt_table) - 1, kernel_idt_table};

void idt_load()
{
    isr_initialize();
    
    kernel_idtr.size = sizeof(kernel_idt_table) - 1;
    kernel_idtr.ptr = kernel_idt_table;
    
    inline_asm(
        "cli\n\t"
        "lidt %0" : : "m"(kernel_idtr) : "memory");
}

void idt_disable_gate(uint32_t interrupt)
{
    FLAG_UNSET(kernel_idt_table[interrupt].flags, IDT_FLAG_PRESENT);
}

void idt_enable_gate(uint32_t interrupt)
{
    FLAG_SET(kernel_idt_table[interrupt].flags, IDT_FLAG_PRESENT);
}

void idt_set_gate(uint32_t interrupt, void *callback, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
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

idt_entry_t *idt_get_entry(uint16_t sel)
{
    return NULL;
}

void idt_dump_selector(uint16_t sel)
{
#ifdef DEBUG
    uint16_t index = sel >> 3;
    idt_dump_selector_index(index);
#endif
}

void idt_dump_selector_index(uint16_t index)
{
#ifdef DEBUG
    idtr_t idtr;
    __asm__("sidt %0" : "=m"(idtr));
    idt_entry_t *idt = (idt_entry_t *)(idtr.ptr);
    idt_entry_t e = idt[index];
#ifdef __i686__
    uint32_t offset = ((uint32_t)e.base_1 << 16) | e.base_2;
#else
    uint64_t offset = ((uint64_t)e.base_1 << 48) | ((uint64_t)e.base_2 << 32) | e.base_3;
#endif
    log_info(MODULE, "idt[%02x]: offset=0x%08X selector=0x%04X type_attr=0x%02X", index, offset, e.segment_selector, e.flags);

#endif
}

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

void IDT_Load(IDT_descriptor* idt_descriptor)
{

}

IDT_entry IDT_table[256];

IDT_descriptor IDT_descript = { sizeof(IDT_table) - 1, IDT_table };

#ifdef __i686__
void x86_IDT_set_gate(int interrupt, void* base, uint32_t segment_descriptor, uint8_t flags)
{
#else
void x86_IDT_set_gate(int interrupt, void* base_ptr, uint32_t segment_descriptor, uint8_t flags, uint8_t ist)
{
    uint64_t base = (uint64_t)base_ptr;
    #endif
    IDT_table[interrupt].base_1 = ((uint32_t)base) & 0xFFFF;
    IDT_table[interrupt].segment_selector = segment_descriptor;
    IDT_table[interrupt].reserved1 = 0;
    IDT_table[interrupt].flags = flags;
    IDT_table[interrupt].base_2 = ((uint32_t)base >> 16) & 0xFFFF;
    #ifdef __x86_64__
    IDT_table[interrupt].ist = ist;
    IDT_table[interrupt].base_3 = ((uint32_t)base >> 16) & 0xFFFF;
#endif
}

void x86_IDT_disable_gate(int interrupt)
{
    FLAG_UNSET(IDT_table[interrupt].flags, IDT_FLAG_PRESENT);
}

void x86_IDT_enable_gate(int interrupt)
{
    FLAG_SET(IDT_table[interrupt].flags, IDT_FLAG_PRESENT);
}


void x86_IDT_load()
{
    __asm__ volatile("lidt %0" : : "m"(IDT_descript));
}

void x86_IDT_initialize()
{

}

// debug function
#ifdef DEBUG

void x86_IDT_dump_selector(uint16_t sel)
{
    IDT_descriptor idtr;
    __asm__ ("sidt %0" : "=m"(idtr));
    IDT_entry* idt = (IDT_entry*)(idtr.ptr);
    IDT_entry e = idt[sel];
    #ifdef __i686__
    uint32_t offset = ((uint32_t)e.base_1 << 16) | e.base_2;
    #else
    uint64_t offset = ((uint64_t)e.base_1 << 48) | ((uint64_t)e.base_2 << 32) | e.base_3;
    #endif
    fprintf(stddebug, "IDT[%02x]: offset=0x%08X selector=0x%04X type_attr=0x%02X",
           sel, offset, e.segment_selector, e.flags);
}

#endif
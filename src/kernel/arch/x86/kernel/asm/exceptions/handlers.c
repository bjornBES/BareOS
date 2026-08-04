/*
 * File: handlers.c
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "exception.h"

#include "kernel/asm/segment/gdt.h"
#include "kernel/asm/ivt/idt.h"

#include "kernel/ivt.h"
#include "kernel.h"

extern void hexdump(void *ptr, int len);

int invalid_opcode_handler(intr_frame_t *frame)
{
    hexdump((void *)frame->pc, 0x40);
    return RETURN_FAILED;
}

int invalid_segment_not_present(intr_frame_t *frame)
{
    // uint8_t external = BIT_GET(frame->error, 0);
    uint8_t table = BIT_GET_RANGE(frame->error, 1, 2);
    uint16_t selector = frame->error & ~0x3;
    if (table == 0b00)
    {
        x86_gdt_dump_selector(selector);
    }
    else if (table == 0b01 || table == 0b11)
    {
        x86_idt_dump_selector(selector);
    }
    hexdump((void *)frame->pc, 0x40);
    return RETURN_FAILED;
}

void handler_init()
{
    ivt_arch_set_handler(EXC_UD, invalid_opcode_handler);
    ivt_arch_set_handler(EXC_NP, invalid_segment_not_present);
}

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
#include "kernel/ivt.h"
#include "kernel.h"

extern void hexdump(void *ptr, int len);
int invalid_opcode_handler(intr_frame_t *frame)
{
    hexdump((void*)frame->pc, 0x40);
    return RETURN_FAILED;
}

void handler_init()
{
    ivt_arch_set_handler(EXC_UD, invalid_opcode_handler);
}
/*
 * File: ivt_arch.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "type_arch.h"

typedef int (*interrupt_handler)(intr_frame_t *regs);

void ivt_arch_init()
{
    return;
}

int ivt_arch_set_handler(interrupt_vector vector, interrupt_handler handler)
{
    return 0;
}

void ivt_arch_enable(interrupt_vector vector)
{
    return;
}

void ivt_arch_disable(interrupt_vector vector)
{
    return;
}

interrupt_vector ivt_find_free_vector(interrupt_vector mask)
{
    return;
}

void ivt_dump_frame(intr_frame_t *frame)
{
    return;
}

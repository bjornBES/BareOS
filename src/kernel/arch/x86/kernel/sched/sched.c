/*
 * File: sched.c
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/sched_arch.h"

#include <defs.h>

NORETURN void scheduler_arch_yield()
{
    inline_asm("int 0x7F");
    while (true)
    {
        ;
    }
    
}
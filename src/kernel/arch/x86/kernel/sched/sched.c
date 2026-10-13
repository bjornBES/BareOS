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

#include "debug/debug.h"

#include <defs.h>

#define MODULE "x86-sched"

NORETURN void scheduler_arch_yield()
{
    ENTER_FUNC("", 0);
    inline_asm("int 0xD1");
    while (true)
    {
        ;
    }
    
}
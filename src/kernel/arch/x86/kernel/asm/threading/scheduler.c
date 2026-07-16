/*
 * File: scheduler.c
 * File Created: 20 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/threading/scheduler.h"
#include "kernel.h"

int scheduler_arch_yield()
{
    __asm__("int 0x7F"); // todo
    return RETURN_GOOD;
}

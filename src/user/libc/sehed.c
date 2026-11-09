/*
 * File: sehed.c
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sched.h"
#include "syscall.h"

int sched_yield()
{
    return syscall0(SYS_YIELD);
}
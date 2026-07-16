/*
 * File: spinlock.c
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "spinlock.h"
#include "task/threading/spinlock/spinlock.h"

void cpu_relax()
{
#ifdef __x86_64__
    __asm__ volatile("pause");
#else
    __asm__ volatile("nop");
#endif
}
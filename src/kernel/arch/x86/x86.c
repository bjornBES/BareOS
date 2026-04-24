/*
 * File: x86.c
 * File Created: 23 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "x86.h"

void arch_breakpoint()
{
    __asm__ __volatile__("int3" : : : "memory");
}
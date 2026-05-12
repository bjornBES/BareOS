/*
 * File: spinlock.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

static inline void cpu_relax()
{
#ifdef __x86_64__
    __asm__ volatile("pause");
#else
    __asm__ volatile("nop");
#endif
}

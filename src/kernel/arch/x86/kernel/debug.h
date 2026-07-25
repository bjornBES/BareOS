/*
 * File: debug.h
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/io.h"
#include "kernel/memory.h"
#include "device/device.h"

static spinlock_t debug_lock = {0};
static inline void e9_putc(char c)
{
    spinlock_acquire(&debug_lock);
    outb(0xE9, c);
    spinlock_release(&debug_lock);
}

void debug_write_char(char c);
size_t debug_write_line(char *buf, size_t count);
void e9_init();
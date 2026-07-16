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

static inline void e9_putc(char c)
{
    outb(0xE9, c);
}
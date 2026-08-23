/*
 * File: io.h
 * File Created: 19 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ ("out dx, al" : : "d" (port), "a"(value));
}
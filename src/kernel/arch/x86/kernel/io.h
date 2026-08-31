/*
 * File: io.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>
#include <types.h>

INLINE void outb(uint16_t port, uint8_t value)
{
    __asm__ ("out dx, al" : : "d" (port), "a"(value));
}
INLINE uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ ("in al, dx" : "=a"(val) : "d" (port));
    return val;
}
INLINE void outw(uint16_t port, uint16_t value)
{
    __asm__ ("out dx, ax" : : "d" (port), "a"(value));
}
INLINE uint16_t inw(uint16_t port)
{
    uint16_t val;
    __asm__ ("in ax, dx" : "=a"(val) : "d" (port));
    return val;
}
INLINE void outd(uint16_t port, uint32_t value)
{
    __asm__ ("out dx, eax" : : "d" (port), "a"(value));
}
INLINE uint32_t ind(uint16_t port)
{
    uint32_t val;
    __asm__ ("in eax, dx" : "=a"(val) : "d" (port));
    return val;
}
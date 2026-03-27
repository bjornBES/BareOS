/*
 * File: IO.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdint.h>

void outb(uint16_t port, uint8_t value)
{
    __asm__ ("out dx, al" : : "d" (port), "a"(value));
}
uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ ("in al, dx" : "=a"(val) : "d" (port));
    return val;
}
void outw(uint16_t port, uint16_t value)
{
    __asm__ ("out dx, ax" : : "d" (port), "a"(value));
}
uint16_t inw(uint16_t port)
{
    uint16_t val;
    __asm__ ("in ax, dx" : "=a"(val) : "d" (port));
    return val;
}
void outd(uint16_t port, uint32_t value)
{
    __asm__ ("out dx, eax" : : "d" (port), "a"(value));
}
uint32_t ind(uint16_t port)
{
    uint32_t val;
    __asm__ ("in eax, dx" : "=a"(val) : "d" (port));
    return val;
}

void enable_interrupts()
{
    __asm__ volatile("sti");
}
void disableInterrupts()
{
    __asm__ volatile("cli");
}
    

void callNMI()
{
    __asm__ volatile("int 0x2");
}

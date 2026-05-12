/*
 * File: pit.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pit.h"

#include "irq.h"
#include "kernel/exceptions/exception.h"
#include "i8259.h"

#include "libs/IO.h"

#include "kernel.h"

#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42
#define PIT_COMMAND_REGISTER 0x43
#define PIT_MODE_REGISTER 0x43

// Command Register Bits [7:6] - Channel Select
#define PIT_CMD_CHANNEL0 0x00 // Select channel 0
#define PIT_CMD_CHANNEL1 0x40 // Select channel 1
#define PIT_CMD_CHANNEL2 0x80 // Select channel 2
#define PIT_CMD_READBACK 0xC0 // Read-back command

// Command Register Bits [5:4] - Access Mode
#define PIT_CMD_LATCH 0x00  // Latch count value
#define PIT_CMD_LOBYTE 0x10 // Access low byte only
#define PIT_CMD_HIBYTE 0x20 // Access high byte only
#define PIT_CMD_LOHI 0x30   // Access low byte then high byte

// Command Register Bits [3:1] - Operating Mode
#define PIT_CMD_MODE0 0x00 // Interrupt on terminal count
#define PIT_CMD_MODE1 0x02 // Hardware retriggerable one-shot
#define PIT_CMD_MODE2 0x04 // Rate generator (divide by N)
#define PIT_CMD_MODE3 0x06 // Square wave generator
#define PIT_CMD_MODE4 0x08 // Software triggered strobe
#define PIT_CMD_MODE5 0x0A // Hardware triggered strobe

// Command Register Bit [0] - BCD/Binary
#define PIT_CMD_BINARY 0x00 // 16-bit binary counter
#define PIT_CMD_BCD 0x01    // BCD counter (do not use)

// PIT base frequency in Hz
#define PIT_BASE_FREQ 1193182

// Read-back command bits (used with PIT_CMD_READBACK)
#define PIT_RB_COUNT 0x20  // Latch count
#define PIT_RB_STATUS 0x10 // Latch status
#define PIT_RB_CH2 0x08    // Read back channel 2
#define PIT_RB_CH1 0x04    // Read back channel 1
#define PIT_RB_CH0 0x02    // Read back channel 0

#define MODULE "PIT"

uint32_64 ticks_timer = 0;

void x86_pit_set_hz(int hz)
{
    int divisor = PIT_BASE_FREQ / hz;                                                             /* Calculate our divisor */
    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LOHI | PIT_CMD_MODE3 | PIT_CMD_BINARY); /* Set our command byte 0x36/0b00110110 */
    outb(PIT_CHANNEL0, divisor & 0xFF);                                                           /* Set low byte of divisor */
    outb(PIT_CHANNEL0, divisor >> 8);                                                             /* Set high byte of divisor */
}

void x86_pit_handler(registers *regs)
{
    ticks_timer++;

    driver->send_eoi(0);
}

void x86_pit_wait(uint32_64 ticks)
{
    ticks_timer = 0;
    while (ticks_timer < ticks)
    {
        continue;
    }
}

uint64_t x86_pit_get_ticks()
{
    return ticks_timer;
}

void x86_pit_init(uint32_t target_hz)
{
    ticks_timer = 0;
    x86_pit_set_hz(target_hz);
    // irq_register_handler(0, x86_pit_handler);
}
/*
 * File: pit.c
 * File Created: 19 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/dev/pit/pit.h"
#include "kernel/irq/irq.h"
#include "kernel/io.h"

#include "irq/irq.h"

#include "timer/timer.h"

#include "dev/device.h"

#include "kerrno.h"

#include <types.h>

#define PIT_CHANNEL0         0x40
#define PIT_CHANNEL1         0x41
#define PIT_CHANNEL2         0x42
#define PIT_COMMAND_REGISTER 0x43
#define PIT_MODE_REGISTER    0x43

// Command Register Bits [7:6] - Channel Select
#define PIT_CMD_CHANNEL0     0x00 // Select channel 0
#define PIT_CMD_CHANNEL1     0x40 // Select channel 1
#define PIT_CMD_CHANNEL2     0x80 // Select channel 2
#define PIT_CMD_READBACK     0xC0 // Read-back command

// Command Register Bits [5:4] - Access Mode
#define PIT_CMD_LATCH        0x00 // Latch count value
#define PIT_CMD_LOBYTE       0x10 // Access low byte only
#define PIT_CMD_HIBYTE       0x20 // Access high byte only
#define PIT_CMD_LOHI         0x30 // Access low byte then high byte

// Command Register Bits [3:1] - Operating Mode
#define PIT_CMD_MODE0        0x00 // Interrupt on terminal count
#define PIT_CMD_MODE1        0x02 // Hardware retriggerable one-shot
#define PIT_CMD_MODE2        0x04 // Rate generator (divide by N)
#define PIT_CMD_MODE3        0x06 // Square wave generator
#define PIT_CMD_MODE4        0x08 // Software triggered strobe
#define PIT_CMD_MODE5        0x0A // Hardware triggered strobe

// Command Register Bit [0] - BCD/Binary
#define PIT_CMD_BINARY       0x00 // 16-bit binary counter
#define PIT_CMD_BCD          0x01 // BCD counter (do not use)

// PIT base frequency in Hz
#define PIT_BASE_FREQ        1193182

// Read-back command bits (used with PIT_CMD_READBACK)
#define PIT_RB_COUNT         0x20 // Latch count
#define PIT_RB_STATUS        0x10 // Latch status
#define PIT_RB_CH2           0x08 // Read back channel 2
#define PIT_RB_CH1           0x04 // Read back channel 1
#define PIT_RB_CH0           0x02 // Read back channel 0

#define MODULE               "PIT"

static timer_callback_t pit_callback;

uint32_64 ticks_timer = 0;

status_t pit_set_oneshot(timer_source_t *self, uint64_t ns, timer_callback_t cb)
{
    pit_callback = cb;
    uint32_t divisor = (PIT_BASE_FREQ * ns) / 1000000000ull;
    if (divisor > 0xFFFF)
    {
        divisor = 0xFFFF;
    }

    SAVE_IRQ();

    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LOHI); // channel 0, oneshot, binary
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    RESTORE_IRQ();
    return KERRNO_SUCCESSES;
}

status_t pit_set_periodic(timer_source_t *self, uint64_t ns, timer_callback_t cb)
{
    pit_callback = cb;
    uint32_t divisor = (PIT_BASE_FREQ * ns) / 1000000000ull;
    if (divisor > 0xFFFF)
    {
        divisor = 0xFFFF;
    }

    SAVE_IRQ();

    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LOHI | PIT_CMD_MODE3 | PIT_CMD_BINARY); // channel 0, periodic, binary
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    RESTORE_IRQ();
    return KERRNO_SUCCESSES;
}

status_t pit_cancel(timer_source_t *self)
{
    SAVE_IRQ();
    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LOHI);
    outb(PIT_CHANNEL0, 0);
    outb(PIT_CHANNEL0, 0);
    RESTORE_IRQ();
    return KERRNO_SUCCESSES;
}

uint64_t pit_read_count(timer_source_t *self)
{
    unsigned count = 0;

    // Disable interrupts
    SAVE_IRQ();

    // al = channel in bits 6 and 7, remaining bits clear
    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LATCH);

    count = inb(PIT_CHANNEL0);       // Low byte
    count |= inb(PIT_CHANNEL0) << 8; // High byte

    RESTORE_IRQ();

    return (uint64_t)count;
}

uint64_t pit_ticks_to_ns(timer_source_t *self, uint64_t ticks)
{
    return (ticks * 1000000000ull) / PIT_BASE_FREQ;
}

status_t pit_irq_handler(intr_frame_t *frame, void *ctx)
{
    irq_eoi(frame->interrupt);
    return KERRNO_SUCCESSES;
}

void pit_init()
{
    device_t *pit = device_create();
    pit->class_name = "pit";
    pit->class = DEVICE_TIMER;
    device_register(pit);

    timer_source_t *pit_timer = timer_create();

    pit_timer->name = "pit";
    pit_timer->dev = pit->devt.id;
    pit_timer->caps.freq_hz = PIT_BASE_FREQ;
    pit_timer->caps.interrupt_capable = 1;
    pit_timer->caps.invariant = 1;
    pit_timer->caps.read_cost_ns = 500000;
    pit_timer->caps.min_interval_ns = 1 / PIT_BASE_FREQ;
    pit_timer->caps.max_interval_ns = UINT16_MAX / PIT_BASE_FREQ;
    pit_timer->read_counter = pit_read_count;
    pit_timer->ticks_to_ns = pit_ticks_to_ns;
    timer_register(pit_timer);

    irq_register_handler(0, pit_irq_handler, NULL, IRQ_TRIGGER_EDGE, IRQ_POLARITY_HIGH, 0);
}

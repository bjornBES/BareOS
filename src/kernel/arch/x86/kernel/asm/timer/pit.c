/*
 * File: pit.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "pit.h"
#include "time/timer.h"
#include "kernel/irq.h"
#include "device/device.h"
#include "kernel/io.h"
#include "kernel/memory.h"

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

static void (*pit_callback)(device_t *) = NULL;

uint32_64 ticks_timer = 0;

int pit_set_oneshot(device_t *dev, uint64_t ns, void (*cb)(device_t *))
{
    pit_callback = cb;
    uint32_t divisor = (PIT_BASE_FREQ * ns) / 1000000000ull;
    if (divisor > 0xFFFF)
    {
        divisor = 0xFFFF;
    }

    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LOHI); // channel 0, oneshot, binary
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    return 0;
}

int pit_set_periodic(device_t *dev, uint64_t ns, void (*cb)(device_t *))
{
    pit_callback = cb;
    uint32_t divisor = (PIT_BASE_FREQ * ns) / 1000000000ull;
    if (divisor > 0xFFFF)
    {
        divisor = 0xFFFF;
    }

    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LOHI | PIT_CMD_MODE3 | PIT_CMD_BINARY); // channel 0, periodic, binary
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    return 0;
}

void pit_cancel(device_t *dev)
{
    outb(PIT_COMMAND_REGISTER, PIT_CMD_CHANNEL0 | PIT_CMD_LOHI);
    outb(PIT_CHANNEL0, 0);
    outb(PIT_CHANNEL0, 0);
}

/* static void pit_irq_handler(intr_frame_t *frame, void *ctx)
{
    if (pit_callback)
    {
        pit_callback(NULL);
    }
    irq_arch_eoi(0);
}
 */
void pit_init()
{
    device_t *pit = malloc(sizeof(device_t));
    timer_priv_t *priv = malloc(sizeof(timer_priv_t));

    priv->name = "pit";
    priv->rating = TIMER_RATING_PIT;
    priv->role = TIMER_ROLE_BOTH;
    priv->is_stable = true;
    priv->is_per_cpu = false;
    priv->read_counter = NULL;
    priv->counter_freq = NULL;
    priv->set_oneshot = pit_set_oneshot;
    priv->set_periodic = pit_set_periodic;
    priv->cancel = pit_cancel;

    pit->class_name = "pit";
    pit->type = DEVICE_TIMER;
    pit->priv = priv;

    device_register(pit);
    timer_register(pit);
    // irq_arch_register(0, pit_irq_handler, NULL);
}

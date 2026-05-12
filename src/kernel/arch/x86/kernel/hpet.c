/*
 * File: hpet.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "hpet.h"

#include "irq.h"
#include "ACPI/ACPI.h"
#include "hal/hal.h"
#include "memory/memdefs.h"
#include "memory/ioremap/ioremap.h"
#include "memory/paging/paging.h"
#include "task/threading/scheduling/scheduler.h"

#define MODULE "hpet"

// timer N register offsets
#define HPET_TIMER_CONFIG(n) (0x100 + n * 0x20)     // config and capabilities
#define HPET_TIMER_COMPARATOR(n) (0x108 + n * 0x20) // comparator value

#define HPET_REG_CAPS 0x00    // capabilities and ID
#define HPET_REG_CONFIG 0x10  // general configuration
#define HPET_REG_COUNTER 0xF0 // main counter value

#define HPET_TIMER_CONFIG_INTENABLE (1 << 2)          // enable interrupt
#define HPET_TIMER_CONFIG_PERIODIC (1 << 3)           // periodic mode
#define HPET_TIMER_CONFIG_PERIODIC_CAP (1 << 4)       // check this first — periodic supported?
#define HPET_TIMER_CONFIG_ROUTE(n) ((uint64_t)n << 9) // GSI route

#define HPET_CONFIG_ENABLE (1 << 0)

typedef struct
{
    SDT_header header;
    uint32_t event_timer_block_id;
    generic_address_structure base_address;
    uint8_t hpet_number;
    uint16_t minimum_clock_tick;
    uint8_t page_protection;
} __attribute__((packed)) HPET_table;

static uint64_t s_hpet_base = 0;
static uint32_t period_fs;
static volatile uint64_t s_ticks = 0;
HPET_table *table;

static uint64_t hpet_read(uint32_t reg)
{
    return *(volatile uint64_t *)(s_hpet_base + reg);
}

static void hpet_write(uint32_t reg, uint64_t value)
{
    *(volatile uint64_t *)(s_hpet_base + reg) = value;
}

time_t HPET_read_counter(void)
{
    return (time_t)hpet_read(HPET_REG_COUNTER);
}

time_t HPET_elapsed_ms(time_t start)
{
    time_t ticks = HPET_read_counter() - start;
    // period is in femtoseconds, 1ms = 1000000000000 fs
    return (time_t)((ticks * period_fs) / 1000000000000ULL);
}
void HPET_sleep_ms(time_t time)
{
    time_t now = HPET_read_counter();
    while (HPET_elapsed_ms(now) < time)
    {
    }
}

void hpet_handler(registers *regs)
{
    s_ticks++;
    // log_info("\0", "HPET");
    schedule(regs);
}

uint64_t HPET_get_ticks(void)
{
    return s_ticks;
}

bool HPET_probe()
{
    table = (HPET_table *)ACPI_get_table("HPET");
    if (!table)
    {
        log_crit(MODULE, "no HPET");
        return false;
    }
    return true;
}

void HPET_init(uint32_t hz)
{
    uint64_t phys = table->base_address.address;
    uint64_t virt = (uint64_t)ioremap((phys_addr)phys, PAGE_SIZE);
    s_hpet_base = virt;

    uint64_t caps = hpet_read(HPET_REG_CAPS);

    // clock period is in femtoseconds, stored in bits [63:32]
    period_fs = caps >> 32;
    uint32_t freq_hz = 1000000000000000ULL / period_fs;

    log_info(MODULE, "period=%u fs freq=%u hz", period_fs, freq_hz);

    // enable the main counter
    hpet_write(HPET_REG_CONFIG, hpet_read(HPET_REG_CONFIG) | HPET_CONFIG_ENABLE);

    uint64_t timer0_cfg = hpet_read(HPET_TIMER_CONFIG(0));

    // check periodic is supported
    if (!(timer0_cfg & HPET_TIMER_CONFIG_PERIODIC_CAP))
    {
        log_warn(MODULE, "timer0 does not support periodic mode");
        return;
    }

    // target 100hz = 10ms period
    // period in ticks = 10ms / period_fs * 1e15
    uint64_t ticks_per_ms = 1000000000000ULL / period_fs;
    uint64_t interval = ticks_per_ms * 10; // 10ms = 100hz

    // disable main counter while programming
    hpet_write(HPET_REG_CONFIG, hpet_read(HPET_REG_CONFIG) & ~HPET_CONFIG_ENABLE);

    // program timer 0
    hpet_write(HPET_TIMER_CONFIG(0),
               HPET_TIMER_CONFIG_INTENABLE |
                   HPET_TIMER_CONFIG_PERIODIC |
                   HPET_TIMER_CONFIG_ROUTE(2)); // GSI 2

    // set comparator — for periodic mode write interval twice
    hpet_write(HPET_TIMER_COMPARATOR(0), hpet_read(HPET_REG_COUNTER) + interval);
    hpet_write(HPET_TIMER_COMPARATOR(0), interval);

    // re-enable main counter
    hpet_write(HPET_REG_CONFIG, hpet_read(HPET_REG_CONFIG) | HPET_CONFIG_ENABLE);

    irq_register_handler(0, hpet_handler);

    log_info(MODULE, "timer0 programmed at %uhz via GSI 2", interval);

    log_info(MODULE, "HPET enabled counter=0x%x", hpet_read(HPET_REG_COUNTER));
}
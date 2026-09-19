/*
 * File: hpet.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/table.h"

#include "asm/irq_arch.h"
#include "irq/irq.h"

#include "debug/debug.h"
#include "dev/device.h"

#include "mm/memdefs.h"
#include "mm/ioremap.h"

#include "timer/timer.h"

#include "memory.h"
#include "math.h"

#include <binary.h>

#define MODULE                         "x86-hpet"

// timer N register offsets
#define HPET_TIMER_CONFIG(n)           (0x100 + 0x20 * n) // config and capabilities
#define HPET_TIMER_COMPARATOR(n)       (0x108 + 0x20 * n) // comparator value

#define HPET_REG_CAPS                  0x00               // capabilities and ID
#define HPET_REG_CONFIG                0x10               // Interrupt Status Register
#define HPET_REG_STATUS                0x20               // general configuration
#define HPET_REG_COUNTER               0xF0               // main counter value

#define HPET_TIMER_CONFIG_INTENABLE    (1 << 2)           // enable interrupt
#define HPET_TIMER_CONFIG_PERIODIC     (1 << 3)           // periodic mode
#define HPET_TIMER_CONFIG_PERIODIC_CAP (1 << 4)           // check this first — periodic supported?
#define HPET_TIMER_CONFIG_ROUTE(n)     ((uint64_t)n << 9) // GSI route

#define HPET_CONFIG_ENABLE             (1 << 0)

#define HPET_MAX_COMPARATORS           32

typedef struct hpet_table
{
    sdt_header_t header;

    uint8_t hardware_rev_id;

    uint8_t comparator_count : 5;
    uint8_t counter_size : 1;
    uint8_t reserved : 1;
    uint8_t legacy_replacement : 1;
    uint16_t pci_vendor_id;

    generic_address_structure base_address;

    uint8_t hpet_number;
    uint16_t minimum_clock_tick;
    uint8_t page_protection;
} PACKED hpet_table_t;

typedef struct
{
    uint8_t index;
    uint8_t irq;
    bool in_use;
    timer_callback_t callback;
} hpet_comparator_t;

static hpet_comparator_t *comparators;
static uint8_t comparator_count = 0;
static volatile vaddr_t hpet_base = 0;
uint64_t hpet_freq = 0;

static uint32_t period_fs;
static volatile uint64_t s_ticks = 0;
hpet_table_t *hpet;

uint64_t hpet_arch_read(uint32_t reg)
{
    uint32_t l = *(volatile uint32_t *)(hpet_base + reg);
    uint32_t h = *(volatile uint32_t *)(hpet_base + reg + 4);
    return (uint64_t)h << 32 | l;
}

static void hpet_write(uint32_t reg, uint64_t value)
{
    *(volatile uint64_t *)(hpet_base + reg) = value;
}

uint64_t hpet_arch_read_counter()
{
    return hpet_arch_read(HPET_REG_COUNTER);
}

uint64_t hpet_read_counter(timer_source_t *dev)
{
    return hpet_arch_read(HPET_REG_COUNTER);
}

uint64_t hpet_ticks_to_ns(timer_source_t *dev, uint64_t ticks)
{
    return (ticks * 1000000000ull) / hpet_freq;
}

static inline hpet_comparator_t *hpet_alloc_comparator()
{
    for (size_t i = 0; i < comparator_count; i++)
    {
        if (comparators[i].in_use == false)
        {
            comparators[i].in_use = true;
            return &comparators[i];
        }
    }
    return NULL;
}

// clock event ops
status_t hpet_set_oneshot(timer_source_t *dev, uint64_t ns, timer_callback_t cb)
{
    log_err(MODULE, "one short");
    hpet_comparator_t *comp = hpet_alloc_comparator();
    if (!comp)
    {
        log_err(MODULE, "ENODEV");
        return -ENODEV;
    }

    comp->callback = cb;

    uint64_t ticks = (ns * hpet_freq) / 1000000000ull;
    uint64_t target = hpet_arch_read(HPET_REG_COUNTER) + ticks;

    // disable comparator, set oneshot, set target, enable
    hpet_write(HPET_TIMER_CONFIG(comp->index), hpet_arch_read(HPET_TIMER_CONFIG(comp->index)) | HPET_TIMER_CONFIG_INTENABLE);
    hpet_write(HPET_TIMER_COMPARATOR(comp->index), target);

    return KERRNO_SUCCESSES;
}

status_t hpet_set_periodic(timer_source_t *dev, uint64_t ns, timer_callback_t cb)
{
    hpet_comparator_t *comp = hpet_alloc_comparator();
    if (!comp)
    {
        return -ENODEV;
    }

    comp->callback = cb;

    uint64_t ticks = (ns * hpet_freq) / 1000000000ull;

    hpet_write(HPET_TIMER_CONFIG(comp->index), hpet_arch_read(HPET_TIMER_CONFIG(comp->index)) | HPET_TIMER_CONFIG_INTENABLE | HPET_TIMER_CONFIG_PERIODIC);
    hpet_write(HPET_TIMER_COMPARATOR(comp->index), ticks);

    return KERRNO_SUCCESSES;
}

status_t hpet_cancel(timer_source_t *dev)
{
    // disable all comparators
    for (int i = 0; i < comparator_count; i++)
    {
        hpet_write(HPET_TIMER_CONFIG(i), 0);
    }
    return KERRNO_SUCCESSES;
}

status_t hpet_irq_handler(intr_frame_t *regs, void *ctx)
{
    log_debug(MODULE, "comparator IRQ fired");
    hpet_comparator_t *comp = (hpet_comparator_t *)ctx;
    hpet_write(HPET_REG_CONFIG, 1 << comp->index);
    if (comp->callback)
    {
        comp->callback(NULL);
    }
    comp->in_use = false;
    irq_eoi(comp->irq);
    return KERRNO_SUCCESSES;
}

extern void hexdump(void *ptr, size_t len, size_t size);

status_t hpet_arch_init(sdt_header_t *hpet_header)
{
    hpet = (hpet_table_t *)hpet_header;

    if (!hpet)
    {
        log_crit(MODULE, "no HPET");
        KERRNO_RETURN(0, ""); // TODO
    }
    hexdump(hpet, sizeof(hpet_table_t), 16);
    log_info(MODULE, "IO address_space = %x", hpet->base_address.address_space);
    log_info(MODULE, "IO access_size = %x", hpet->base_address.access_size);
    log_info(MODULE, "IO address = %p", hpet->base_address.address);
    volatile paddr_t phys = hpet->base_address.address;
    volatile vaddr_t virt = ioremap(phys, 4096);
    hpet_base = virt;
    log_info(MODULE, "mapped IO address from %p to %p", phys, virt);

    uint64_t caps = hpet_arch_read(HPET_REG_CAPS);
    if (!caps)
    {
        KERRNO_RETURN(0, "caps 0x%016x are not valid", caps); // TODO
    }
    log_info(MODULE, "caps=0x%llx", caps);

    // clock period is in femtoseconds, stored in bits [63:32]
    period_fs = caps >> 32;
    hpet_freq = 1000000000000000ull / period_fs;

    log_info(MODULE, "period=%u fs freq=%u hz", period_fs, hpet_freq);

    // enable the main counter
    hpet_write(HPET_REG_CONFIG, hpet_arch_read(HPET_REG_CONFIG) | HPET_CONFIG_ENABLE);

    uint64_t timer0_cfg = hpet_arch_read(HPET_TIMER_CONFIG(0));

    // check periodic is supported
    if (!(timer0_cfg & HPET_TIMER_CONFIG_PERIODIC_CAP))
    {
        KERRNO_RETURN(0, "timer0 does not support periodic mode"); // TODO
    }

    // enumerate comparators
    comparator_count = BIT_GET_RANGE(caps, 8, 12);
    log_debug(MODULE, "hpet->comparator_count = %u", hpet->comparator_count);
    log_debug(MODULE, "comparator_count = %u", comparator_count);
    comparators = kcalloc(comparator_count, sizeof(hpet_comparator_t));
    for (int i = 0; i < comparator_count; i++)
    {
        uint64_t timer_cfg = hpet_arch_read(0x100 + 0x20 * i);
        log_debug(MODULE, "comparator %u cfg = 0x%llx", i, timer_cfg);
        uint32_t irq_mask = BIT_GET_RANGE(timer_cfg, 32, 63);

        log_debug(MODULE, "comparator has mask 0x%x", irq_mask);
        gsi_t gsi = irq_pick_free_gsi(irq_mask);
        log_debug(MODULE, "comparator %u got gsi%u", i, gsi);

        comparators[i].index = i;
        comparators[i].irq = gsi;
        comparators[i].in_use = false;

        // route comparator to IRQ
        uint64_t expected_value = hpet_arch_read(HPET_TIMER_CONFIG(i)) | (gsi << 9);
        hpet_write(HPET_TIMER_CONFIG(i), expected_value);

        log_debug(MODULE, "%x == %x", BIT_GET_RANGE(hpet_arch_read(HPET_TIMER_CONFIG(i)), 9, 13), gsi);

        irq_register_handler(gsi, hpet_irq_handler, &comparators[i], IRQ_TRIGGER_EDGE, IRQ_POLARITY_HIGH, 0);

        if (BIT_GET(timer_cfg, 5) == 1)
        {
            // timer has 64 bit
            BIT_UNSET(timer_cfg, 8); // disable forced 32 bit mode
        }
    }

    device_t *hpet = device_create();
    hpet->class_name = "hpet";
    hpet->class = DEVICE_TIMER;
    // hpet->hpet_timer = hpet_timer;
    device_register(hpet);
    timer_source_t *hpet_timer = timer_create();
    timer_source_t *hpet_main_timer = timer_create();

    hpet_timer->name = "hpet";
    hpet_timer->caps.interrupt_capable = 1;
    hpet_timer->caps.supports_oneshot = 1;
    hpet_timer->caps.supports_periodic = 1;
    hpet_timer->caps.read_cost_ns = 100000;
    hpet_timer->caps.freq_hz = hpet_freq;
    hpet_timer->caps.min_interval_ns = 1;
    hpet_timer->caps.max_interval_ns = UINT32_MAX / hpet_freq;

    hpet_timer->arm_oneshot = hpet_set_oneshot;
    hpet_timer->arm_periodic = hpet_set_periodic;
    hpet_timer->cancel = hpet_cancel;
    hpet_timer->dev = hpet->devt.id;

    hpet_main_timer->name = "hpet-main";
    hpet_main_timer->caps.invariant = 1;
    hpet_main_timer->caps.read_cost_ns = 100000;
    hpet_main_timer->caps.freq_hz = hpet_freq;
    hpet_main_timer->caps.min_interval_ns = 1;
    hpet_main_timer->caps.max_interval_ns = UINT64_MAX / hpet_freq;

    hpet_main_timer->read_counter = hpet_read_counter;
    hpet_main_timer->ticks_to_ns = hpet_ticks_to_ns;
    hpet_main_timer->dev = hpet->devt.id;

    hpet_write(HPET_REG_CONFIG, hpet_arch_read(HPET_REG_CONFIG) | HPET_CONFIG_ENABLE);
    hpet_write(HPET_REG_COUNTER, 0);

    timer_register(hpet_timer);
    timer_register(hpet_main_timer);
    return KERRNO_SUCCESSES;
}

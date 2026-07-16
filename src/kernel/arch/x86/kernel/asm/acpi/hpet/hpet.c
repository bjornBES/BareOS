/*
 * File: hpet.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "hpet.h"
#include "time/timer.h"
#include "acpi/table.h"

#include "kernel/irq.h"
#include "kernel/memory.h"
#include "time/timer.h"
#include "device/device.h"
#include "mm/memdefs.h"
#include "mm/ioremap/ioremap.h"
#include "task/threading/scheduling/scheduler.h"
#include "errno/errno.h"

#include "math.h"

#define MODULE "hpet"

// timer N register offsets
#define HPET_TIMER_CONFIG(n) (0x100 + n * 0x20)       // config and capabilities
#define HPET_TIMER_COMPARATOR(n) (0x108 + n * 0x20)   // comparator value

#define HPET_REG_CAPS 0x00                            // capabilities and ID
#define HPET_REG_STATUS 0x20                          // general configuration
#define HPET_REG_CONFIG 0x10                          // Interrupt Status Register
#define HPET_REG_COUNTER 0xF0                         // main counter value

#define HPET_TIMER_CONFIG_INTENABLE (1 << 2)          // enable interrupt
#define HPET_TIMER_CONFIG_PERIODIC (1 << 3)           // periodic mode
#define HPET_TIMER_CONFIG_PERIODIC_CAP (1 << 4)       // check this first — periodic supported?
#define HPET_TIMER_CONFIG_ROUTE(n) ((uint64_t)n << 9) // GSI route

#define HPET_CONFIG_ENABLE (1 << 0)

#define HPET_MAX_COMPARATORS 32

typedef struct
{
    sdt_header header;

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
} __attribute__((packed)) HPET_table;

typedef struct
{
    uint8_t index;
    uint8_t irq;
    bool in_use;
    void (*callback)(device_t *);
} hpet_comparator_t;

static hpet_comparator_t *comparators;
static uint8_t comparator_count = 0;
static volatile uint64_t hpet_base = 0;
static uint64_t hpet_freq = 0;

static uint32_t period_fs;
static volatile uint64_t s_ticks = 0;
HPET_table *table;

static uint64_t hpet_read(uint32_t reg)
{
    return *(volatile uint64_t *)(hpet_base + reg);
}

static void hpet_write(uint32_t reg, uint64_t value)
{
    *(volatile uint64_t *)(hpet_base + reg) = value;
}

uint64_t hpet_read_counter(device_t *dev)
{
    return hpet_read(HPET_REG_COUNTER);
}

uint64_t hpet_get_freq(device_t *dev)
{
    return hpet_freq;
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
static int hpet_set_oneshot(device_t *dev, uint64_t ns, void (*cb)(device_t *))
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
    uint64_t target = hpet_read(HPET_REG_COUNTER) + ticks;

    // disable comparator, set oneshot, set target, enable
    hpet_write(HPET_TIMER_CONFIG(comp->index), hpet_read(HPET_TIMER_CONFIG(comp->index)) | HPET_TIMER_CONFIG_INTENABLE);
    hpet_write(HPET_TIMER_COMPARATOR(comp->index), target);

    return 0;
}

int hpet_set_periodic(device_t *dev, uint64_t ns, void (*cb)(device_t *))
{
    hpet_comparator_t *comp = hpet_alloc_comparator();
    if (!comp)
    {
        return -ENODEV;
    }

    comp->callback = cb;

    uint64_t ticks = (ns * hpet_freq) / 1000000000ull;

    hpet_write(HPET_TIMER_CONFIG(comp->index), hpet_read(HPET_TIMER_CONFIG(comp->index)) | HPET_TIMER_CONFIG_INTENABLE | HPET_TIMER_CONFIG_PERIODIC);
    hpet_write(HPET_TIMER_COMPARATOR(comp->index), ticks);

    return 0;
}

void hpet_cancel(device_t *dev)
{
    // disable all comparators
    for (int i = 0; i < comparator_count; i++)
    {
        hpet_write(HPET_TIMER_CONFIG(i), 0);
    }
}

void hpet_irq_handler(intr_frame_t *regs, void *ctx)
{
    // log_debug("HPET", "comparator IRQ fired");    // ← add this
    hpet_comparator_t *comp = (hpet_comparator_t *)ctx;
    hpet_write(HPET_REG_CONFIG, 1 << comp->index);
    if (comp->callback)
    {
        comp->callback(NULL);
    }
    comp->in_use = false;
    irq_arch_eoi(comp->irq);
}

int hpet_init()
{
    table = (HPET_table *)table_get_table(0x48504554);
    if (!table)
    {
        log_crit(MODULE, "no HPET");
        return RETURN_FAILED;
    }
    log_info(MODULE, "IO address_space = %x", table->base_address.address_space);
    log_info(MODULE, "IO access_size = %x", table->base_address.access_size);
    log_info(MODULE, "IO address = %p", table->base_address.address);
    volatile paddr_t phys = table->base_address.address;
    volatile vaddr_t virt = ioremap(phys, PAGE_SIZE);
    hpet_base = virt;
    log_info(MODULE, "mapped IO address from %p to %p", phys, virt);

    uint64_t caps = hpet_read(HPET_REG_CAPS);
    if (!caps)
    {
        return RETURN_FAILED;
    }
    
    // clock period is in femtoseconds, stored in bits [63:32]
    period_fs = caps >> 32;
    hpet_freq = 1000000000000000ull / period_fs;

    log_info(MODULE, "period=%u fs freq=%u hz", period_fs, hpet_freq);

    // enable the main counter
    hpet_write(HPET_REG_CONFIG, hpet_read(HPET_REG_CONFIG) | HPET_CONFIG_ENABLE);

    uint64_t timer0_cfg = hpet_read(HPET_TIMER_CONFIG(0));

    // check periodic is supported
    if (!(timer0_cfg & HPET_TIMER_CONFIG_PERIODIC_CAP))
    {
        log_warn(MODULE, "timer0 does not support periodic mode");
        return RETURN_FAILED;
    }

    // enumerate comparators
    comparator_count = (hpet_read(HPET_REG_CAPS) >> 8) & 0x1F;
    comparator_count = min(comparator_count, table->comparator_count);
    comparators = calloc(comparator_count, sizeof(hpet_comparator_t));
    log_debug(MODULE, "comparator_count = %u", comparator_count);
    for (int i = 1; i < comparator_count; i++)
    {
        uint32_t irq_mask = hpet_read(HPET_TIMER_CONFIG(i)) >> 32;
        uint32_t irq = irq_pick_free_irq(irq_mask);

        comparators[i].index = i;
        comparators[i].irq = irq;
        comparators[i].in_use = false;

        // route comparator to IRQ
        hpet_write(HPET_TIMER_CONFIG(i), hpet_read(HPET_TIMER_CONFIG(i)) | (irq << 9));

        irq_arch_register(irq, hpet_irq_handler, &comparators[i]);
    }

    device_t *hpet = malloc(sizeof(device_t));
    timer_priv_t *priv = malloc(sizeof(timer_priv_t));
    
    priv->name = "hpet";
    priv->rating = TIMER_RATING_HPET;
    priv->role = TIMER_ROLE_BOTH;
    priv->is_stable = true;
    priv->is_per_cpu = false;
    priv->read_counter = hpet_read_counter;
    priv->counter_freq = hpet_get_freq;
    priv->set_oneshot = hpet_set_oneshot;
    priv->set_periodic = hpet_set_periodic;
    priv->cancel = hpet_cancel;
    
    hpet->class_name = "hpet";
    hpet->type = DEVICE_TIMER;
    hpet->priv = priv;
    
    hpet_write(HPET_REG_CONFIG, hpet_read(HPET_REG_CONFIG) | HPET_CONFIG_ENABLE);

    device_register(hpet);
    timer_register(hpet);
    return RETURN_GOOD;
}

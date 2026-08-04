/*
 * File: lapci.c
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "lapic.h"
#include "device/device.h"
#include "time/timer.h"
#include "kernel/cpu.h"
#include "kernel/ivt.h"
#include "kernel/irq.h"
#include "kernel/asm/acpi/hpet/hpet.h"

#include "errno/errno.h"

#define MODULE            "LAPIC"

#define TIMER_SCALE_SHIFT 32
#define SCALE_FACTOR      (1ull << TIMER_SCALE_SHIFT) // 2^32

typedef struct lapic_timer_priv
{
    cpu_id id;
    void (*callback)(device_t *dev);
    uint64_t ticks_per_ns_scaled; // calibrated once globally, cached here per-core for cheap use
} lapic_timer_priv_t;

void lapic_enable()
{
    // enable local APIC, set spurious vector to 0xFF
    lapic_write(LAPIC_REG_SPURIOUS,
                lapic_read(LAPIC_REG_SPURIOUS) | LAPIC_SPURIOUS_ENABLE | 0xFF);
}

void lapic_wait_idle()
{
    while (lapic_read(LAPIC_REG_ICR_LOW) & (1 << 12));
}

lapic_id lapic_get_id()
{
    return (lapic_id)(lapic_read(LAPIC_REG_ID) >> 24);
}

// LVT Timer register bit layout (same reg you already write in lapic_timer_init):
//   bits 0-7   : vector
//   bit  16    : mask (1 = masked)
//   bit  17    : mode (0 = one-shot, 1 = periodic)

static int lapic_timer_set_oneshot(device_t *_, uint64_t ns, void (*cb)(device_t *))
{
    device_t *dev = cpu_arch_get_current()->lapic_timer_dev;
    timer_priv_t *priv = dev->priv;
    lapic_timer_priv_t *lapic_priv = priv->priv;
    lapic_priv->callback = cb;

    uint64_t ticks = (ns * lapic_priv->ticks_per_ns_scaled) >> TIMER_SCALE_SHIFT;

    // one-shot mode: clear periodic bit, keep vector, unmask
    lapic_write(LAPIC_REG_TIMER, LAPIC_TIMER_VECTOR); // mode=0, mask=0
    lapic_write(LAPIC_REG_TIMER_INITIAL, ticks);

    return 0;
}

int lapic_timer_set_periodic(device_t *dev, uint64_t ns, void (*cb)(device_t *))
{
    ENTER_FUNC(MODULE, "%p, %u, %p", dev, ns, cb);
    timer_priv_t *priv = dev->priv;
    lapic_timer_priv_t *lapic_priv = priv->priv;
    lapic_priv->callback = cb;
    lapic_priv->id = cpu_arch_get_current()->apic_id;
    log_debug(MODULE, "setting periodic on cpu %u", cpu_arch_get_current()->apic_id);
    
    uint64_t ticks = (ns * lapic_priv->ticks_per_ns_scaled) >> TIMER_SCALE_SHIFT;
    
    // periodic mode: set mode bit once, initial count auto-reloads every period —
    // unlike HPET, no per-tick rewrite of the comparator needed
    lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);
    lapic_write(LAPIC_REG_TIMER, LAPIC_TIMER_PERIODIC | LAPIC_TIMER_VECTOR);
    lapic_write(LAPIC_REG_TIMER_INITIAL, ticks);
    return 0;
}

void lapic_timer_cancel(device_t *dev)
{
    // mask the LVT entry — don't just zero the count, a tick can still land
    // mid-reprogram and fire the old callback on the new state
    lapic_write(LAPIC_REG_TIMER, LAPIC_TIMER_VECTOR | LAPIC_TIMER_MASKED);
    lapic_write(LAPIC_REG_TIMER_INITIAL, 0);
}

uint64_t calibrate_lapic_timer()
{
    // TODO: read CPUID.06H:EAX
    const uint64_t CAL_WINDOW_NS = 10000000ull; // 10ms window, plenty for precision

    // mask timer, set divide (already done in lapic_timer_init, but be explicit here)
    lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);
    lapic_write(LAPIC_REG_TIMER, LAPIC_TIMER_VECTOR | LAPIC_TIMER_MASKED);
    lapic_write(LAPIC_REG_TIMER_INITIAL, 0xFFFFFFFF); // count down from max, masked so no IRQ fires

    uint64_t hpet_ticks_needed = (CAL_WINDOW_NS * hpet_freq) / 1000000000ull;
    uint64_t hpet_start = hpet_read(HPET_REG_COUNTER);

    while (hpet_read(HPET_REG_COUNTER) - hpet_start < hpet_ticks_needed); // busy wait — this only runs once at boot, per core

    uint32_t remaining = lapic_read(LAPIC_REG_TIMER_CURRENT);
    uint64_t lapic_ticks_elapsed = 0xFFFFFFFFull - remaining;

    // ticks_per_ns, scaled up by 2^32 to keep precision as an integer
    uint64_t ticks_per_ns_scaled = (lapic_ticks_elapsed * SCALE_FACTOR) / CAL_WINDOW_NS;

    lapic_write(LAPIC_REG_TIMER_INITIAL, 0); // stop it, don't leave it running

    return ticks_per_ns_scaled;
}

int lapic_timer_isr(intr_frame_t *frame)
{
    cpu_t *cpu = cpu_arch_get_current();
    device_t *dev = cpu->lapic_timer_dev;
    timer_priv_t *priv = dev->priv;
    lapic_timer_priv_t *lapic_priv = priv->priv;

    if (lapic_priv->callback)
    {
        // log_debug(MODULE, "apic_id = %u", lapic_priv->id);
        lapic_priv->callback(dev);
    }

    lapic_eoi();

    return RETURN_GOOD;
}

int ipi_reschedule_handler(intr_frame_t *frame)
{
    cpu_arch_get_current()->need_resched = true;
    lapic_eoi();
    return RETURN_GOOD;
}

void lapic_timer_init(uint64_t calibrated)
{
    ENTER_FUNC(MODULE, "%u", calibrated);
    // set divide config to 16
    lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);

    lapic_write(LAPIC_REG_TIMER_INITIAL, 0);

    // set timer to periodic mode, vector 0x40
    lapic_write(LAPIC_REG_TIMER, LAPIC_TIMER_PERIODIC | LAPIC_TIMER_VECTOR);


    device_t *lapic = malloc(sizeof(device_t));
    timer_priv_t *priv = malloc(sizeof(timer_priv_t));
    lapic_timer_priv_t *lapic_priv = malloc(sizeof(lapic_timer_priv_t));

    priv->name = "lapic";
    priv->rating = TIMER_RATING_LAPIC;
    priv->role = TIMER_ROLE_EVENT;
    priv->is_stable = true;
    priv->is_per_cpu = true;
    priv->read_counter = NULL;
    priv->counter_freq = NULL;
    priv->set_oneshot = lapic_timer_set_oneshot;
    priv->set_periodic = lapic_timer_set_periodic;
    priv->cancel = lapic_timer_cancel;

    lapic_priv->ticks_per_ns_scaled = calibrated;
    lapic_priv->id = cpu_arch_get_current()->apic_id;

    priv->priv = lapic_priv;

    lapic->class_name = "lapic";
    lapic->type = DEVICE_TIMER;
    lapic->priv = priv;

    cpu_arch_get_current()->lapic_timer_dev = lapic;

    device_register(lapic);
    timer_register(lapic);
}

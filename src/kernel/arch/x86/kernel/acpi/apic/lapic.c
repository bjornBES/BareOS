/*
 * File: lapic.c
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/acpi/apic/lapic.h"

#include "kernel/cpuid/cpuid.h"

#include "kernel/tsc/tsc.h"
#include "kernel/msr/msr.h"

#include "x86_arch_data.h"

#include "dev/device.h"
#include "timer/timer.h"

#include "asm/cpu_arch.h"
#include "asm/hpet_arch.h"
#include "asm/vectors_arch.h"

#include "debug/debug.h"

#include "memory.h"

#include <binary.h>
#include <defs.h>

#define MODULE "x86-lapic"

typedef struct lapic_timer_priv
{
    timer_callback_t callback;
} lapic_timer_priv_t;

vaddr_t local_apic_base;
extern x86_arch_data_t arch_runtime_data;

inline void lapic_write_icr(uint32_t high, uint32_t low)
{
    if (arch_runtime_data.cpuid.leaf_0x1_0->x2apic)
    {
        wrmsr(0x830, ((uint64_t)high << 32) | low);
        return;
    }
    *(volatile uint32_t *)(local_apic_base + LAPIC_REG_ICR_HIGH) = (uint32_t)high << 24;
    *(volatile uint32_t *)(local_apic_base + LAPIC_REG_ICR_LOW) = (uint32_t)low;
}

inline void lapic_write(uint32_t reg, uint64_t value)
{
    if (arch_runtime_data.cpuid.leaf_0x1_0->x2apic)
    {
        wrmsr(0x800 + (reg >> 4), value);
        return;
    }
    *(volatile uint32_t *)(local_apic_base + reg) = (uint32_t)value;
}

inline uint64_t lapic_read(uint32_t reg)
{
    if (arch_runtime_data.cpuid.leaf_0x1_0->x2apic)
    {
        return rdmsr(0x800 + (reg >> 4));
    }
    return (uint64_t)*(volatile uint32_t *)(local_apic_base + reg);
}

inline uint32_t lapic_get_id()
{
    if (arch_runtime_data.cpuid.leaf_0x1_0->x2apic)
    {
        uint32_t low;
        uint32_t high;
        msr_get_32(0x802, &low, &high);
        return low;
    }
    else if (arch_runtime_data.cpuid.leaf_0x1_0->apic)
    {
        return BIT_GET_RANGE(*(volatile uint32_t *)(local_apic_base + LAPIC_REG_ID), 24, 31);
    }
    return BIT_GET_RANGE(*(volatile uint32_t *)(local_apic_base + LAPIC_REG_ID), 24, 27);
}

void lapic_enable()
{
    // enable local APIC, set spurious vector to 0xFF
    lapic_write(LAPIC_REG_SPURIOUS, lapic_read(LAPIC_REG_SPURIOUS) | LAPIC_SPURIOUS_ENABLE | 0xFF);
}

void lapic_wait_idle()
{
    while (lapic_read(LAPIC_REG_ICR_LOW) & (1 << 12));
}

uint64_t lapic_timer_get_freq()
{
    uint64_t tsc_freq = 0;
    uint64_t tsc_start = 0;
    uint32_t remaining = 0;
    uint64_t lapic_ticks_elapsed = 0;

    cpuid_regs regs;
    cpuid(0x80000007, 0x0, &regs);
    if (BIT_GET(regs.edx, 8) == 1)
    {
        cpuid(0x0, 0x0, &regs);
        uint32_t max_leaf = regs.eax;
        if (max_leaf >= 0x15)
        {
            cpuid(0x15, 0x0, &regs);
            uint32_t denominator = regs.eax; // CPUID.15H:EAX — denominator of TSC/crystal ratio
            uint32_t numerator = regs.ebx;   // CPUID.15H:EBX — numerator of TSC/crystal ratio
            uint32_t crystal_hz = regs.ecx;  // CPUID.15H:ECX — core crystal clock frequency in Hz

            if (numerator != 0 && denominator != 0 && crystal_hz != 0)
            {
                // TSC freq = crystal_freq * (numerator / denominator)
                tsc_freq = ((uint64_t)crystal_hz * numerator) / denominator;
            }
            else
            {
                // TODO use CPUID.0x01 family/model
                FUNC_NOT_IMPLEMENTED();
            }
        }
        else
        {
            uint64_t hpet_start = hpet_arch_read_counter();
            tsc_start = rdtsc();

            // busy-wait until some fixed HPET duration has elapsed, e.g. 10ms
            uint64_t hpet_ticks_wanted = hpet_freq / 100; // 10ms worth of ticks
            while ((hpet_arch_read_counter() - hpet_start) < hpet_ticks_wanted)
            {
                inline_asm("pause");
            }

            uint64_t tsc_end = rdtsc();
            uint64_t hpet_end = hpet_arch_read_counter();

            uint64_t tsc_delta = tsc_end - tsc_start;
            uint64_t hpet_delta = hpet_end - hpet_start;

            // tsc_freq = tsc_delta / (hpet_delta / hpet_freq)
            tsc_freq = (tsc_delta * hpet_freq) / hpet_delta;
        }
    }
    else
    {
        const uint64_t CAL_WINDOW_NS = 10000000ull; // 10ms window, plenty for precision

        lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);
        lapic_write(LAPIC_REG_TIMER_INITIAL, 0xFFFFFFFF); // count down from max, masked so no IRQ fires

        uint64_t hpet_ticks_needed = (CAL_WINDOW_NS * hpet_freq) / 1000000000ull;
        uint64_t hpet_start = hpet_arch_read_counter();

        while (hpet_arch_read_counter() - hpet_start < hpet_ticks_needed)
        {
            // busy wait — this only runs once at boot, per core
        }

        remaining = lapic_read(LAPIC_REG_TIMER_CURRENT);
        lapic_ticks_elapsed = 0xFFFFFFFFull - remaining;

        lapic_write(LAPIC_REG_TIMER_INITIAL, 0); // stop it, don't leave it running

        return lapic_ticks_elapsed * 100;
    }

    // Set divide configuration first (e.g. divide-by-16)
    lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);

    tsc_start = rdtsc();
    lapic_write(LAPIC_REG_TIMER_INITIAL, 0xFFFFFFFF); // count down from max, one-shot mode implied

    // wait a fixed TSC duration, e.g. equivalent to 10ms using tsc_freq
    uint64_t tsc_wait = tsc_freq / 100;
    while ((rdtsc() - tsc_start) < tsc_wait)
    {
    }

    remaining = lapic_read(LAPIC_REG_TIMER_CURRENT); // Current Count Register
    lapic_ticks_elapsed = 0xFFFFFFFF - remaining;

    lapic_write(LAPIC_REG_TIMER_INITIAL, 0);

    // lapic_freq (post-divider) = ticks_elapsed / (10ms)
    return lapic_ticks_elapsed * 100; // ticks per second
}

// LVT Timer register bit layout (same reg you already write in lapic_timer_init):
//   bits 0-7   : vector
//   bit  16    : mask (1 = masked)
//   bit  17    : mode (0 = one-shot, 1 = periodic)

static int lapic_timer_set_oneshot(timer_source_t *_, uint64_t ns, timer_callback_t cb)
{
    timer_source_t *self = cpu_arch_get_current()->cpu_timer_dev;
    lapic_timer_priv_t *lapic_priv = self->priv;
    
    lapic_priv->callback = cb;
    uint64_t ticks = (ns * self->caps.freq_hz);

    // one-shot mode: clear periodic bit, keep vector, unmask
    lapic_write(LAPIC_REG_TIMER, CPU_TIMER_VECTOR); // mode=0, mask=0
    lapic_write(LAPIC_REG_TIMER_INITIAL, ticks);

    return 0;
}

int lapic_timer_set_periodic(timer_source_t *_, uint64_t ns, timer_callback_t cb)
{
    cpu_t *cpu = cpu_arch_get_current();
    timer_source_t *self = cpu->cpu_timer_dev;
    ENTER_FUNC("%p, %u, %p", self, ns, cb);
    lapic_timer_priv_t *lapic_priv = self->priv;

    lapic_priv->callback = cb;
    trace_debug(MODULE, "setting periodic on cpu %u", cpu->arch_id);
    
    uint64_t ticks = (ns * self->caps.freq_hz);
    
    // periodic mode: set mode bit once, initial count auto-reloads every period
    lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);
    lapic_write(LAPIC_REG_TIMER, LAPIC_TIMER_PERIODIC | CPU_TIMER_VECTOR);
    lapic_write(LAPIC_REG_TIMER_INITIAL, ticks);
    trace_debug(MODULE, "done here");
    return 0;
}

status_t lapic_timer_cancel(timer_source_t *_)
{
    timer_source_t *self = cpu_arch_get_current()->cpu_timer_dev;
    lapic_timer_priv_t *lapic_priv = self->priv;
    
    lapic_priv->callback = NULL;

    // mask the LVT entry don't just zero the count, a tick can still land
    // mid-reprogram and fire the old callback on the new state
    lapic_write(LAPIC_REG_TIMER, CPU_TIMER_VECTOR | LAPIC_TIMER_MASKED);
    lapic_write(LAPIC_REG_TIMER_INITIAL, 0);
    return KERRNO_SUCCESSES;
}

status_t lapic_timer_init(uint32_t lapic_id, cpu_logical_id_t logical_id)
{
    uint64_t lapic_freq = lapic_timer_get_freq();

    cpu_t *cpu = cpu_arch_get(logical_id);

    timer_source_t *lapic_timer = timer_create();
    cpu->cpu_timer_dev = lapic_timer;
    lapic_timer->name = "lapic";
    lapic_timer->caps.per_cpu = 1;
    lapic_timer->caps.interrupt_capable = 1;
    lapic_timer->caps.supports_oneshot = 1;
    lapic_timer->caps.supports_periodic = 1;
    lapic_timer->caps.read_cost_ns = 10000;
    lapic_timer->caps.freq_hz = lapic_freq;
    lapic_timer->caps.min_interval_ns = 1;
    lapic_timer->caps.max_interval_ns = UINT32_MAX / lapic_freq;
    
    lapic_timer->arm_oneshot = lapic_timer_set_oneshot;
    lapic_timer->arm_periodic = lapic_timer_set_periodic;
    lapic_timer->cancel = lapic_timer_cancel;

    lapic_timer->priv = kmalloc(sizeof(lapic_timer_priv_t));

    if (lapic_id == cpu_arch_get_bsp()->arch_id)
    {
        device_t *lapic = device_create();
        lapic->class_name = "lapic";
        lapic->class = DEVICE_TIMER;
        device_register(lapic);
        timer_register(lapic_timer);
    }
    


    return KERRNO_SUCCESSES;
}


/*
 * File: panic.c
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "panic.h"

#include "debug/debug.h"

#include "smp/smp.h"
#include "cpu/cpu.h"

#include <init.h>
#include <types.h>

atomic_int panic_cpu;

status_t panic_init()
{
    atomic_init(&panic_cpu, INVALID_CPU);

    return KERRNO_SUCCESS;
}

EARLY_INITCALL(panic_init);

bool panic_on_this_cpu()
{
    return __atomic_load_n(&panic_cpu, __ATOMIC_ACQUIRE) == cpu_this_id();
}

bool panic_in_progress()
{
	return UNLIKELY(atomic_load(&panic_cpu) != INVALID_CPU);
}

bool panic_on_other_cpu()
{
	return (panic_in_progress() && !panic_on_this_cpu());
}

void kernel_panic(char *file, int line, const char *function, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int expected = -1;
    if (!atomic_compare_exchange_strong(&panic_cpu, &expected, cpu_this_id()))
    {
        if (expected == cpu_this_id())
        {
            cpu_halt_forever();
        }
        cpu_halt_forever();
    }

    smp_send_ipi_others(IPI_CPUS_STOP);

    console_write(CON_WRITE_LEVEL_PANIC, LVL_EMERG, "KERNEL PANIC\n%s:%i in function %s", file, line, function);
    console_write_arg(CON_WRITE_LEVEL_PANIC, LVL_INFO, fmt, args);

    va_end(args);

    // TODO: make it reboot the system
    for (;;)
    {
        ;
    }
}

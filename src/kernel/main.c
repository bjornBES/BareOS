/*
 * File: main.c
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "init.h"

#include "acpi/fadt/fadt.h"

#include "dev/device.h"
#include "timer/timer.h"

#include "smp/smp.h"

#include "debug/debug.h"

#include "thread/thread.h"
#include "sched/sched.h"

#include <boot/params.h>

#define MODULE "main"

void kernel_main(boot_params_t *boot_params)
{
    ENTER_FUNC("%p", boot_params);
    for (;;)
    {
        ;
    }
}

THREAD_WARPER_NO_RETURN(kernel_main, boot_params_t *);

__init void kernel_early_main(boot_params_t *boot_params)
{
    smp_init(boot_params);

    fadt_parse();

    trace_info(NO_MODULE, "timer_now_ticks() = %lld", timer_now_ticks());
    trace_info(NO_MODULE, "timer_now_ns()    = %lld", timer_now_ns());

    device_debug();

    thread_t *main_thread = thread_create_kernel(THREAD_CALL_FUNC(kernel_main), (uintptr_t)boot_params, 0);
    sched_init(main_thread);

    trace_info(MODULE, "jump to kernel_main");

    schedule(NULL);
    
    for (;;)
    {
        ;
    }
}
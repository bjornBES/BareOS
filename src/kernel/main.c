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

#include <boot/params.h>

void kernel_main(boot_params_t *boot_params)
{

    for (;;)
    {
        ;
    }
}

__init void kernel_early_main(boot_params_t *boot_params)
{
    smp_init(boot_params);

    fadt_parse();

    for (size_t i = 0; i < 500000000; i++)
    {
        ;
    }
    

    log_info(NO_MODULE, "timer_now_ticks() = %lld", timer_now_ticks());
    log_info(NO_MODULE, "timer_now_ns() = %lld", timer_now_ns());

    device_debug();
    for (;;)
    {
        ;
    }
}
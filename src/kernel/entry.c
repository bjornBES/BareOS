/*
 * File: entry.c
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "init.h"

#include <boot/params.h>
#include "setup.h"

#include "mm/pmm/pmm.h"
#include "mm/ioremap.h"

#include "drivers/base/driver.h"
#include "drivers/base/bus.h"

#include "cmdline/cmdline.h"

#include "ivt/ivt.h"

#include "stdio.h"
#include "debug/debug.h"

#define MODULE "entry"

NORETURN __init void kernel_entry(boot_params_t *boot_params)
{
    cmdline_early_init(boot_params);

    extern char __initcall_start;
    initcall_t *funcs = (initcall_t *)&__initcall_start;
    while (*funcs != NULL)
    {
        trace_debug(MODULE, "init func %p", *funcs);
        status_t ret = (*funcs)();
        if (ret != KERRNO_SUCCESSES)
        {
            KERNEL_PANIC(MODULE, "init function at %p returned non zero", *funcs);
        }
        funcs++;
    }

    pmm_early_init(boot_params);
    
    arch_setup(boot_params);

    for (;;)
    {
        ;
    }
}

status_t entry_init_param(const char *value)
{
    ENTER_FUNC("%p (%s)", value, value);
    return KERRNO_SUCCESSES;
}
CMDLINE_PARAM("test", entry_init_param, 1);
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

#include "drivers/driver.h"
#include "drivers/bus.h"

#include "ivt/ivt.h"

#include "stdio.h"
#include "debug/debug.h"

void hexdump(void *ptr, size_t len, size_t size)
{
    fprintf(DEBUG_FD, "========= HEXDUMP =========\n");
    fprintf(DEBUG_FD, "hexdump at %p length %u\n", ptr, len);
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i % size) == 0)
        {
            fprintf(DEBUG_FD, "\n%04x: ", i);
        }
        fprintf(DEBUG_FD, "%02x ", p[i]);
    }
    fprintf(DEBUG_FD, "\n");
}

NORETURN __init void kernel_entry(boot_params_t *boot_params)
{
    extern char __initcall_start;
    initcall_t *funcs = (initcall_t *)&__initcall_start;
    while (*funcs != NULL)
    {
        trace_debug("entry", "init func %p", *funcs);
        status_t ret = (*funcs)();
        if (ret != KERRNO_SUCCESSES)
        {
            KERNEL_PANIC("entry", "init function at %p returned non zero", *funcs);
        }
        funcs++;
    }

    pmm_early_init(boot_params);
    
    // TODO maybe also call Core and PostCore around here?

    arch_setup(boot_params);

    for (;;)
    {
        ;
    }
}
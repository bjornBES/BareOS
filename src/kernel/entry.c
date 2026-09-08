/*
 * File: entry.c
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include <boot/params.h>
#include "setup.h"

#include "mm/pmm/pmm.h"

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

NORETURN void kernel_entry(boot_params_t *boot_params)
{
    pmm_early_init(boot_params);

    arch_setup(boot_params);

    for (;;)
    {
        ;
    }
}
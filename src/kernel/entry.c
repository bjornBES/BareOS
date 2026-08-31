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

NORETURN void kernel_entry(boot_params_t *boot_params)
{
    arch_setup(boot_params);

    for (;;)
    {
        ;
    }
}
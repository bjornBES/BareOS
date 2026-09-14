/*
 * File: main.c
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/fadt/fadt.h"

#include <boot/params.h>

void kernel_main(boot_params_t *boot_params)
{
    fadt_parse();
    for (;;)
    {
        ;
    }
}
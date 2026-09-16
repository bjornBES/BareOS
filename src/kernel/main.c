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
#include "acpi/hpet/hpet.h"

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
    fadt_parse();

    hpet_parse();
    for (;;)
    {
        ;
    }
}
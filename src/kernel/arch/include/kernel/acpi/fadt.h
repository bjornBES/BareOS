/*
 * File: fadt.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "acpi/tables/fadt.h"

void fadt_arch_init(fadt_t *fadt);
void fadt_arch_shutdown(fadt_t *fadt);
void fadt_arch_reboot(fadt_t *fadt);
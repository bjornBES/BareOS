/*
 * File: madt.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "acpi/tables/madt.h"

void madt_arch_parse(madt *madt);

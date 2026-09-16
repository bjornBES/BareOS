/*
 * File: hpet_arch.h
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "acpi/table.h"

status_t hpet_init(sdt_header_t *hpet_header);
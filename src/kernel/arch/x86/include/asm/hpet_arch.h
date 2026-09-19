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

extern uint64_t hpet_freq;

uint64_t hpet_arch_read(uint32_t reg);
uint64_t hpet_arch_read_counter();
status_t hpet_arch_init(sdt_header_t *hpet_header);
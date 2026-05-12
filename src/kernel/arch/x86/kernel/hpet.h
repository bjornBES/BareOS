/*
 * File: hpet.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "kernel.h"

uint64_t HPET_get_ticks();
uint64_t HPET_elapsed_ms(time_t start);
void HPET_sleep_ms(time_t start);
bool HPET_probe();
void HPET_init(uint32_t hz);
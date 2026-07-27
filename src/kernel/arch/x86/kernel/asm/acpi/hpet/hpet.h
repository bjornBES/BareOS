/*
 * File: hpet.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "kernel.h"

#define HPET_REG_COUNTER 0xF0                         // main counter value

extern uint64_t hpet_freq;

uint64_t hpet_read(uint32_t reg);

int hpet_pre_init();
int hpet_init();
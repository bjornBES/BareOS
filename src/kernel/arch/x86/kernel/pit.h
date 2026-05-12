/*
 * File: pit.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel.h"

void x86_pit_init(uint32_t hz);
uint64_t x86_pit_get_ticks();
void x86_pit_wait(uint32_64 ticks);
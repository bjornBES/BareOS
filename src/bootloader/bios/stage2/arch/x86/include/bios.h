/*
 * File: bios.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "bios/bios_regs.h"
#include <stdint.h>

void bios_arch_init_regs(bios_regs_t *regs);
void bios_arch_intcall(uint8_t int_no, bios_regs_t *ireg, bios_regs_t *oreg);
void bios_arch_dump_frame(bios_regs_t *frame);

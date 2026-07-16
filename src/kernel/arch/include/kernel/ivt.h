/*
 * File: ivt.h
 * File Created: 01 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "ctx.h"

void ivt_arch_init();

void ivt_arch_set_handler(uint32_t vector, int (*handler)(intr_frame_t *));

void ivt_dump_frame(intr_frame_t *frame);

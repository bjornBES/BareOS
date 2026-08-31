/*
 * File: ivt.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "type_arch.h"

int ivt_arch_handler(interrupt_vector vector, intr_frame_t *frame);
void ivt_dump_frame(intr_frame_t *frame);
/*
 * File: exception.h
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "isr.h"
#include "vectors.h"


typedef int (*arch_handler)(intr_frame_t *regs);

void exception_register_arch_handler(int interrupt, arch_handler handler);
void exception_init();

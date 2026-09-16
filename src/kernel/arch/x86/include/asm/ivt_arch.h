/*
 * File: ivt_arch.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

/// @file This is a test

#include "type_arch.h"

#include "ivt/ivt.h"

typedef int (*interrupt_handler)(intr_frame_t *regs);

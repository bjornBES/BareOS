/*
 * File: signal_types.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "kernel/asm/exceptions/isr.h"


typedef struct signal_arch_frame
{
    intr_frame_t proc_reg;
    sigset_t saved_mask;
} signal_arch_frame_t;

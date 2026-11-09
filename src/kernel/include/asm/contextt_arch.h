/*
 * File: contextt_arch.h
 * File Created: 21 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "type_arch.h"

#include <types.h>

typedef union context
{
    vaddr_t sp;
    intr_frame_t *regs;
} context_t;

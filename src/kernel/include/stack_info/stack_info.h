/*
 * File: stack_info.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "mm/mmu.h"
#include <types.h>

typedef struct stack_info
{
    mmu_flags_t mapping_flags;
    paddr_t stack_top;
    paddr_t stack_bottom;
    size_t stack_size;
    uint8_t has_guard_page : 1;
    uint8_t resv : 7;
} stack_info_t;

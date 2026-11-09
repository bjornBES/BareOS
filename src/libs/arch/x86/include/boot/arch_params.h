/*
 * File: arch_params.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct arch_params
{
    bool disabled_pic;
    bool in_long_mode;
    bool using_paging;
} arch_params_t;

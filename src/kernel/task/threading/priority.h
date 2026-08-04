/*
 * File: priority.h
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

#define PRIORITY_L8            255
#define PRIORITY_L7            224
#define PRIORITY_L6            192

#define PRIORITY_L5            160
#define PRIORITY_L4            128

#define PRIORITY_L3            96
#define PRIORITY_L2            64

#define PRIORITY_L1            32
#define PRIORITY_L0            0

#define PRIORITY_HIGHEST       PRIORITY_L8
#define PRIORITY_HIGH_SHORT    PRIORITY_L7
#define PRIORITY_HIGH          PRIORITY_L6
#define PRIORITY_NORMAL_SHORT  PRIORITY_L5
#define PRIORITY_NORMAL        PRIORITY_L4
#define PRIORITY_MIDDLE_SHORT  PRIORITY_L3
#define PRIORITY_MIDDLE        PRIORITY_L2
#define PRIORITY_LOW_SHORT     PRIORITY_L1
#define PRIORITY_LOW           PRIORITY_L0

#define PRIORITY_MIN_TIMESLICE 5
#define PRIORITY_MAX_TIMESLICE 50

static inline uint32_t priority_to_timeslice(uint8_t priority)
{
    // higher priority = shorter slice = more responsive scheduling
    return PRIORITY_MAX_TIMESLICE -
        ((uint32_t)priority * (PRIORITY_MAX_TIMESLICE - PRIORITY_MIN_TIMESLICE)) / 255;
}

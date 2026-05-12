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

#define PRIORITY_HIGH    255
#define PRIORITY_NORMAL  128
#define PRIORITY_LOW     0

#define PRIORITY_MIN_TIMESLICE  5
#define PRIORITY_MAX_TIMESLICE  50

static inline uint32_t priority_to_timeslice(uint8_t priority)
{
    // maps 0-255 priority to PRIORITY_MIN_TIMESLICE-PRIORITY_MAX_TIMESLICE ticks
    return PRIORITY_MIN_TIMESLICE +
           ((uint32_t)priority * (PRIORITY_MAX_TIMESLICE - PRIORITY_MIN_TIMESLICE)) / 255;
}

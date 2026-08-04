/*
 * File: CMOS.h
 * File Created: 29 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#include "kernel.h"
#include <types.h>

extern int century_register;

void cmos_get_currect_datetime(time_struct_t *out);
time_t cmos_read_time();

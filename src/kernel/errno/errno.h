/*
 * File: errno.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <base_errno.h>
#include "debug/debug.h"

#define PRINT_ERROR(number) log_err(MODULE, "func %s outputting %u", __FUNCTION__, number)
#define SET_ERROR(number) PRINT_ERROR(number)

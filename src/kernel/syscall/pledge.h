/*
 * File: pledge.h
 * File Created: 15 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <pledge_types.h>

int pledge_check(pledge_flags_t function_pledge);
char * pledge_get_missing(pledge_flags_t function_pledge);

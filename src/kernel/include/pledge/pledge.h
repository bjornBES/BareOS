/*
 * File: pledge.h
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <pledge/pledge_uapi.h>
#include <types.h>

status_t pledge_check(pledge_flags_t function_pledge);
char *pledge_get_missing(pledge_flags_t function_pledge);
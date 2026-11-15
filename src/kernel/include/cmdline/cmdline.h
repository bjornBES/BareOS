/*
 * File: cmdline.h
 * File Created: 15 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "cmdlinet.h"
#include "cmdline_init.h"

#include <boot/params.h>

#include <types.h>

status_t cmdline_early_init(boot_params_t *boot_params);

status_t cmdline_get_u64(const char *key, uint64_t *out_num);
status_t cmdline_get_str(const char *key, const char **out_str);
status_t cmdline_has(const char *key);

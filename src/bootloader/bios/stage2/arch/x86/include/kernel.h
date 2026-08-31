/*
 * File: kernel.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <boot/params.h>

typedef void (*boot_start)(boot_params_t *boot);

void enter_kernel(boot_params_t *boot, boot_start *entry);
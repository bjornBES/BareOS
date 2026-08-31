/*
 * File: paging.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <boot/params.h>

void paging32_init(boot_params_t *bp);
void paging64_init(boot_params_t *bp);
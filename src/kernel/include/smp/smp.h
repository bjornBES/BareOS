/*
 * File: smp.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <boot/params.h>

status_t smp_init(boot_params_t *bp);

status_t smp_call_function(cpu_logical_id_t id, void (*func)(uintptr_t), uintptr_t arg);
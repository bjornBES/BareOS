/*
 * File: syscall_arch.h
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "type_arch.h"


#include <boot/params.h>
#include <types.h>


status_t syscall_arch_init(boot_params_t *bp);
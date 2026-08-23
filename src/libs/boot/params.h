/*
 * File: params.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "boot/arch_params.h"

typedef struct boot_params
{
    arch_params_t arch;
} boot_params_t;

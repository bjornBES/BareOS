/*
 * File: acpi.h
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <boot/params.h>
#include <types.h>

/// @brief 
/// @param bp 
/// @return 
int rsdt_parse(boot_params_t *bp);

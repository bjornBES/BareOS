/*
 * File: ioremap.h
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

/// @brief 
void ioremap_init();

/// @brief 
/// @param[in] phys
/// @param[in] size 
/// @return 
vaddr_t ioremap(paddr_t phys, size_t size);
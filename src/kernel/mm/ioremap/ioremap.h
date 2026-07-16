/*
 * File: ioremap.h
 * File Created: 06 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

void ioremap_init();

vaddr_t ioremap(paddr_t phys, size_t size);
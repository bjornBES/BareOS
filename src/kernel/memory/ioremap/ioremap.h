/*
 * File: ioremap.h
 * File Created: 06 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

void ioremap_init();

virt_addr ioremap(phys_addr phys, size_t size);
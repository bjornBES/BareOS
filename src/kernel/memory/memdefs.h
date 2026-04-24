/*
 * File: memdefs.h
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"

#define MEMORY_MIN (virt_addr)0x0000000000000000
#define MEMORY_MAX (virt_addr)0xFFFFFFFFFFFFFFFFULL

#define MEMORY_KERNEL_VIRT_BASE KERNEL_VIRT_BASE

#define MEMORY_HEAP_VIRT_BASE (virt_addr)0xFFFFFFFF00000000ULL

#define MEMORY_STACKS_VIRT_BASE (virt_addr)0xFFFFFFFE00000000ULL

#define MEMORY_MMIO_VIRT_BASE (virt_addr)0xFFFFFFFC00000000ULL

#define MEMORY_DIRECT_MAP_VIRT_BASE 0xFFFF800000000000ULL

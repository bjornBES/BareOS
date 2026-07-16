/*
 * File: mmu.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define PAGE_SIZE       0x1000      // 4KB
#define PAGE_SIZE_LARGE 0x200000    // 2MB
#define PAGE_SIZE_HUGE  0x40000000  // 1GB

#define PAGE_SHIFT      12          // log2(PAGE_SIZE)
#define PAGE_MASK       (~(PAGE_SIZE - 1))
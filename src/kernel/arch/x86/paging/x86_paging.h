/*
 * File: x86_paging.h
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "debug/debug.h"
#include "libs/memory.h"
#include "kernel.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <core/Defs.h>
#include <util/binary.h>
#include <boot/bootparams.h>

#ifdef __i686__
#include "paging_32.h"
#else
#include "paging_64.h"
#endif

#include "memory/paging/paging.h"

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_PWT (1 << 3)
#define PAGE_PCD (1 << 4)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_DIRTY (1 << 6)
#define PAGE_HUGE_PAGES (1 << 7)
#define PDE_GLOBAL (1 << 7)

#define paging_native_flush_tlb_single(addr) \
    __asm__ volatile("invlpg [%0]" :: "r"((uintptr_t)(addr)) : "memory")

void paging_init32(boot_params *boot, size_t kernel_size, phys_addr pmm_start, phys_addr pmm_end);
void paging_init64(boot_params *boot, size_t kernel_size, phys_addr pmm_start, phys_addr pmm_end);

void paging_x86_init(boot_params *boot_params, phys_addr pmm_start, phys_addr pmm_end);
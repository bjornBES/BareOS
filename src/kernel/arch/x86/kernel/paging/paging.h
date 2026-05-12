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
#include "paging_types.h"

#ifdef __i686__
#include "paging_32.h"
#else
#include "paging_64.h"
#endif

#include "debug/debug.h"
#include "libs/memory.h"
#include "memory/flags.h"
#include "kernel.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <defs.h>
#include <util/binary.h>
#include <boot/bootparams.h>

#define paging_native_flush_tlb_single(addr) \
    __asm__ volatile("invlpg [%0]" ::"r"((uintptr_t)(addr)) : "memory")


void arch_paging_init(boot_params *boot_params, phys_addr pmm_start, phys_addr pmm_end);
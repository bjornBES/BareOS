/*
 * File: frame_allocator.h
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/mmu_arch.h"

#include <boot/params.h>
#include <types.h>
#include <config.h>

typedef struct frame_allocator frame_allocator_t;
typedef struct frame_allocator_stats frame_allocator_stats_t;

typedef struct allocator_info
{
    void *priv;
} allocator_info_t;

typedef struct pmm_zone_info
{
    memory_entry_t *memory;
    paddr_t phys_start;
    paddr_t phys_end;

    memory_zone_t zone;

    vaddr_t virt_start;
    vaddr_t virt_end;

    frame_allocator_t *allocator;
    allocator_info_t allocator_info;
} pmm_zone_info_t;

typedef struct frame_allocator_ops
{
    status_t (*alloc)(allocator_info_t *allocator, size_t nframes, paddr_t *phys_out);
    
    status_t (*alloc_at)(allocator_info_t *allocator, paddr_t at_phys, size_t nframes, paddr_t *phys_out);

    status_t (*free)(allocator_info_t *allocator, paddr_t phys, size_t nframes);

    void (*stats)(allocator_info_t *allocator, struct frame_allocator_stats *out);

    void (*print_stat)(allocator_info_t *allocator);
    void (*print_stat_verbose)(allocator_info_t *allocator);
} frame_allocator_ops_t;

typedef struct frame_allocator
{
    const char *name;
    status_t (*setup)(allocator_info_t *allocator, const pmm_zone_info_t *info);
    status_t (*initialize)(allocator_info_t *allocator, const pmm_zone_info_t *info);
    frame_allocator_ops_t ops;
    void *priv;
} frame_allocator_t;

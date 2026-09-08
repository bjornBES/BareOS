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

#include <types.h>
#include <config.h>

struct frame_allocator;
struct frame_allocator_stats;


typedef struct frame_allocator_ops
{
    int (*alloc)(struct frame_allocator *allocator, size_t nframes, paddr_t *phys_out);
    
    int (*alloc_at)(struct frame_allocator *allocator, paddr_t at_phys, size_t nframes, paddr_t *phys_out);

    int (*free)(struct frame_allocator *allocator, paddr_t phys, size_t nframes);

    void (*stats)(struct frame_allocator *allocator, struct frame_allocator_stats *out);

    void (*print_stat)(struct frame_allocator *allocator);
    void (*print_stat_verbose)(struct frame_allocator *allocator);
} frame_allocator_ops_t;

typedef struct frame_allocator
{
    const char *name;
    frame_allocator_ops_t ops;
    void *priv;
} frame_allocator_t;

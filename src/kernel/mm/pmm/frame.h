/*
 * File: frame.h
 * File Created: 18 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#include "kernel.h"

#define FRAME_BITMAP_SIZE (size_t)&__frame_bitmap_size
#define FRAME_BITMAP_WORDS (FRAME_BITMAP_SIZE / sizeof(uint32_t))

extern char __frame_bitmap_size;

/// @brief Get the allocation status of a physical frame.
/// @param frame_index The index of the frame in the physical frame bitmap.
/// @return 0 if free, nonzero if used.
int frame_get_frame(uint32_t frame_index);

/// @brief Mark a physical frame as used in the bitmap.
///
/// @param frame_index The index of the frame to mark in the bitmap.
void frame_mark_frame_used(uint32_t frame_index);

/// @brief Mark a physical frame as free in the bitmap.
///
/// @param frame_index The index of the frame to unmark in the bitmap.
void frame_mark_frame_free(uint32_t frame_index);

/// @brief Allocate a range of physical frames as used.
///
/// @param start_phys Start address of the region (inclusive).
/// @param end_phys End address of the region (exclusive).
void frame_alloc_region(phys_addr start_phys, phys_addr end_phys);

/// @brief Allocate a specific physical frame.
///
/// @param phys_addr Physical address of the frame to allocate.
void frame_alloc_at(phys_addr addr);

/// @brief Allocate the next free physical frame.
///
/// @return Physical address of the allocated frame or 0xFFFFFFFF on failure.
phys_addr frame_alloc_frame();

/// @brief Free a previously allocated physical frame.
///
/// @param phys_addr Physical address of the frame to free.
void frame_free_frame(phys_addr addr);

/// @brief Print the physical frame bitmap to the debug console.
void frame_dump_bitmap();

void frame_dump_bitmap_usage();

/// @brief Initialize frame allocator structures and mark reserved frames.
void frame_init();

void frame_map();
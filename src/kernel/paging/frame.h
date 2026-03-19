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

int frame_get_frame(uint32_t frame_index);
void frame_mark_frame_used(uint32_t frame_index);
void frame_mark_frame_free(uint32_t frame_index);
void frame_alloc_region(uint32_t start_phys, uint32_t end_phys);
void frame_alloc_at(uint32_t phys_addr);
uint32_t frame_alloc_frame();
void frame_free_frame(uint32_t phys_addr);
void frame_dump_bitmap();
void frame_init();

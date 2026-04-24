/*
 * File: memory.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "stdint.h"

typedef struct {
    uint16_t offset;
    uint16_t segment;
} seg_off_t;


void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);

void* segoffset_to_linear(void* addr);
void *segoffset_to_linear_real(uint16_t seg, uint16_t off);
seg_off_t linear_to_segoff(void* linear);
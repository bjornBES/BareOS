/*
 * File: idat.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "sync/spinlock.h"

#include <bitops.h>

#include <types.h>

#define IDA_CHUNK_SIZE   128 /* 128 bytes per chunk */
#define IDA_BITMAP_LONGS BITS_TO_LONGS(IDA_CHUNK_SIZE)
#define IDA_BITMAP_BITS  (IDA_BITMAP_LONGS * sizeof(uint32_64_t) * 8)
#define IDA_PAGE_OFFSET  IDA_BITMAP_BITS

typedef struct ida_page
{
    uint32_64_t bitmap[IDA_BITMAP_LONGS];
    uint32_64_t next_index;
    uint32_64_t offset;
    size_t count;
    uint8_t is_full : 1;
} ida_page_t;

typedef struct ida
{
    ida_page_t **pages;
    uint32_64_t page;
    size_t page_count;
} ida_t;

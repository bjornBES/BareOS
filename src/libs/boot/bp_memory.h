/*
 * File: bp_memory.h
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */



#pragma once

#include <stdint.h>
#include <defs.h>

#define MEMORY_AVAILABLE 1
#define MEMORY_RESERVED 2
#define MEMORY_BAD_RAM 5

typedef struct memory_entry
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t ext_data;
} PACKED memory_entry_t;

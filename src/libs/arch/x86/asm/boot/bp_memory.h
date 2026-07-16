/*
 * File: bp_memory.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#include "bp_config.h"

#define MEMORY_AVAILABLE 1
#define MEMORY_RESERVED 2
#define MEMORY_BAD_RAM 5

typedef struct memory_entry
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
} __attribute__((packed)) memory_entry_t;

typedef struct bp_memory
{
    uint32_t count;
    uint32_t max_count;
    memory_entry_t entries[MAX_MEMORY_ENTRIES]; // MAX_MEMORY_ENTRIES = 32
} __attribute__((packed)) bp_memory_t;

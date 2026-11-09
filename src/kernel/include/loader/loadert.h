/*
 * File: loadert.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "mm/mmut.h"
#include "lists/list_node.h"

#include <types.h>

typedef enum loader_abi
{
    Undefined,
    ABI_SYSV64,
    ABI_SYSV32,
} loader_abi_t;

typedef enum loader_section_type
{
    LOADER_TYPE_TEXT,
    LOADER_TYPE_DATA,
    LOADER_TYPE_RODATA,
    LOADER_TYPE_BSS,
    LOADER_TYPE_HEAP,
    LOADER_TYPE_STACK,
    LOADER_TYPE_NOTE,
} loader_section_type_t;

typedef struct loader_section
{
    LIST_ENTRY_THIS();
    vaddr_t base;
    size_t size;
    loader_section_type_t type;
    mmu_flags_t flags;
} loader_section_t;

typedef struct loader_data
{
    vaddr_t entry;
    list_t sections;

    // the abi the program is using
    loader_abi_t abi;
} loader_data_t;

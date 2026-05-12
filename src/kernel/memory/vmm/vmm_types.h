/*
 * File: vmm_types.h
 * File Created: 06 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "memory/paging/paging_type.h"
#include "memory/flags.h"

typedef enum vma_type
{
    VMA_ANONYMOUS,  // malloc heap, stack, zeroed pages
    VMA_PROC,       // memory space of the process
    VMA_FILE,       // mmap'd file (later)
    VMA_DEVICE,     // MMIO / device mapping (later)
    VMA_SHARED,     // shared memory between processes (later)
} vma_type_t;

typedef struct vma
{
    virt_addr start; // inclusive
    virt_addr end;   // exclusive
    virt_addr top;
    memory_flags_t flags;
    uint32_t type;
    struct vma *next; // intrusive linked list
    struct vma *prev;
} vma_t;

typedef struct vma_memory
{
    vma_t *mmap; // head of VMA list
    size_t map_count;

    virt_addr start_stack;
    virt_addr start_heap;
    virt_addr end_heap;

    paging_page_t page_directory; // page global directory (your CR3 target)
} vma_memory_t;

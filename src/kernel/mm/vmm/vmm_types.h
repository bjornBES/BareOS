/*
 * File: vmm_types.h
 * File Created: 06 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "kernel/mmu.h"

typedef enum vma_type
{
    VMA_ANONYMOUS,   // malloc heap, stack, zeroed pages eager: yes, CoW: yes
    VMA_PROC_TEXT,   // memory space of the process eager: yes, CoW: yes
    VMA_PROC_DATA,   // memory space of the process eager: yes, CoW: yes
    VMA_FILE_PRIV,   // mmap'd file eager: yes, CoW: yes
    VMA_FILE_SHARED, // mmap'd file eager: yes, CoW: yes
    VMA_DEVICE,      // MMIO / device mapping eager: no, CoW: no
    VMA_SHARED,      // shared memory between processes eager: no, CoW: no
} vma_type_t;

typedef struct vma
{
    vaddr_t start;     // range start inclusive
    vaddr_t end;       // range end exclusive
    vaddr_t top;       // currect top
    mmu_flags_t flags; // flags
    uint32_t type;     // type
    struct vma *next;  // intrusive linked list
    struct vma *prev;
} vma_t;

typedef struct vma_memory
{
    vma_t *mmap; // head of VMA list
    size_t map_count;

    vaddr_t start_stack;
    vaddr_t start_heap;
    vaddr_t end_heap;

    vma_t *stack_vma;
    vma_t *heap_vma;

    page_table_t *page_directory; // page global directory (your CR3 target)
} vma_memory_t;

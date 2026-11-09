/*
 * File: vmat.h
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
#include "sync/spinlock.h"

#include "vfs/vfs_internals.h"

#include <types.h>

typedef enum vma_type
{
    VMA_ANONYMOUS,   // malloc heap, stack, zeroed pages
    VMA_PROC_TEXT,   // memory space of the process
    VMA_PROC_DATA,   // memory space of the process
    VMA_FILE_PRIV,   // mmap'd file
    VMA_FILE_SHARED, // mmap'd file
    VMA_DEVICE,      // MMIO / device mapping
    VMA_SHARED,      // shared memory between processes
} vma_type_t;

typedef struct file_ref
{
    refcount_t refcount; // how many VMAs point at this open file
    vfs_node_t *fd;
    spinlock_t lock;   // protects refcount inc/dec across concurrent faults + fork
} file_ref_t;

typedef struct vma vma_t;

typedef struct vma
{
    vaddr_t start;     // range start inclusive
    vaddr_t end;       // range end exclusive
    vaddr_t top;       // currect top
    mmu_flags_t flags; // flags

    file_ref_t *file;
    off_t file_offset; // offset into file corresponding to vaddr_start
    size_t file_size;  // how many bytes are backed by file

    uint32_t populated_pages;

    uint32_t type;    // type

    vma_t *next;
    vma_t *prev;
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
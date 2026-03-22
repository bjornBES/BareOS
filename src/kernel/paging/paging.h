/*
 * File: paging.h
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

/*
 * File: paging.h
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "frame.h"
#include "task/process.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <core/Defs.h>

extern bool paging_print_out;

#define KERNEL_VIRT_BASE 0xC0000000
#define KERNEL_PHYS_BASE 0x00100000 // 1MB

#define PAGE_SIZE 4096

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_PCD (1 << 4)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_DIRTY (1 << 6)
#define PDE_4KB 0x0

#define GETPAGEDIRECTORYINDEX(addr) (uint32_t)addr >> 22
#define GETPAGETABLEINDEX(addr) (uint32_t)addr >> 12 & 0x3FF

typedef union
{
    struct __attribute__((packed))
    {
        union
        {
            struct __attribute__((packed))
            {
                uint32_t present : 1;  /** Set -> present in memory. */
                uint32_t writable : 1; /** Set -> user writable. (read/write bit) */
                uint32_t user : 1;     /** Set -> user accessible. */
                uint32_t cache : 1;    /** Set -> cache disable. */
                uint32_t unused0 : 1;  /** Unused caching bits. */
                uint32_t accessed : 1; /** Set -> accessed sinced mapped. */
                uint32_t dirty : 1;    /** Set -> page has been written to. */
                uint32_t unused1 : 1;  /** Unused bit. */
            } flags_bitmap;
            uint8_t flags_byte;
        } flags;
        uint32_t unused2 : 4;  /** Unused 5 misc bits. */
        uint32_t frame : 20;   /** Physical frame number of the page. */
    };
    uint32_t raw;
} __attribute__((packed)) page_table_entry;
typedef struct
{
    page_table_entry entries[1024];
} page_table;
typedef union
{
    struct __attribute__((packed))
    {
        union
        {
            struct __attribute__((packed)) 
            {
                uint32_t present : 1;  /** Set -> present in memory. */
                uint32_t writable : 1; /** Set -> user writable. (read/write bit) */
                uint32_t user : 1;     /** Set -> user accessible. */
                uint32_t cache : 1;    /** Set -> cache disable. */
                uint32_t unused0 : 1;  /** Unused caching bits. */
                uint32_t accessed : 1; /** Set -> accessed sinced mapped. */
                uint32_t unused1 : 1;  /** Unused bit. */
                uint32_t size : 1;     /** 0 -> using 4KiB page size. */
            } flags_bitmap;
            uint8_t flags_byte;
        } flags;
        
        uint32_t unused2 : 4; /** Unused 4 misc bits. */
        uint32_t frame : 20;  /** Physical frame number of level-2 table. */
    };
    uint32_t raw;
} __attribute__((packed)) page_directory_entry;
typedef struct page_directory_t
{
    page_directory_entry entries[1024];
} page_directory;

extern page_directory *kernel_page_directory;

void *phys_to_virt(void *phys);

void paging_create_user_directory(void *_proc);

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
void *paging_get_physical(page_directory *page_dir, void *virtAddr);

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
void *paging_get_virtual(page_directory *page_dir, void *physAddr);

static inline uint32_t phys_to_page_index(void *physAddr);

static inline void *page_index_to_phys(uint32_t index);

// Allocate any free physical frame. Returns its physical address, or NULL on OOM.
// Does NOT map it into any address space — caller maps it where needed.
void *paging_alloc_frame();

// Allocate the specific physical frame containing physAddr.
// Returns physAddr (page-aligned) on success, NULL if already in use.
void *paging_alloc_frame_at(void *physAddr);

// Free a physical frame by its physical address.
void paging_free_frame(void *physAddr);

// Map a single virtual page → physical frame in the current page directory.
// flags: -1 for PAGE_PRESENT | PAGE_WRITABLE
void paging_map_page(page_directory *page_dir, void *virtAddr, void *physAddr, uint32_t flags);

// Unmap a single virtual page (does NOT free the underlying frame).
void paging_unmap_page(page_directory *page_dir, void *virtAddr);

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
void *paging_alloc_and_map(page_directory *page_dir, void *virtAddr, uint32_t flags);

void *paging_alloc_and_map_region(page_directory *page_dir, void *virtAddr, size_t size, uint32_t flags);

// Map a contiguous physical region [physAddr, physAddr + size) to
// [virtAddr, virtAddr + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
void paging_map_region(page_directory *page_dir, void *virtAddr, void *physAddr, size_t size, uint32_t flags);

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(page_directory *page_dir, void *virtAddr, size_t size);

void paging_init();
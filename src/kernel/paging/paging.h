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
#include <stdint.h>
#include <stddef.h>
#include <core/Defs.h>

#define PAGE_SIZE 4096

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_ACCESSED (1 << 5)
#define PAGE_DIRTY (1 << 6)
#define PDE_4KB 0x0

#define GETPAGEDIRECTORYINDEX(addr) (uint32_t)addr >> 22
#define GETPAGETABLEINDEX(addr) (uint32_t)addr >> 12 & 0x3FF

typedef struct
{
    union
    {
        struct
        {
            uint32_t present : 1;  /** Set -> present in memory. */
            uint32_t writable : 1; /** Set -> user writable. (read/write bit) */
            uint32_t user : 1;     /** Set -> user accessible. */
            uint32_t unused0 : 2;  /** Unused 2 caching bits. */
            uint32_t accessed : 1; /** Set -> accessed sinced mapped. */
            uint32_t dirty : 1;    /** Set -> page has been written to. */
            uint32_t unused1 : 1;  /** Unused bit. */
        } __attribute__((packed)) flags_bitmap;
        uint8_t flags_byte;
    } flags;
    uint32_t unused2 : 4;  /** Unused 5 misc bits. */
    uint32_t frame : 20;   /** Physical frame number of the page. */
} __attribute__((packed)) page_table_entry;
typedef struct
{
    union
    {
        struct 
        {
            uint32_t present : 1;  /** Set -> present in memory. */
            uint32_t writable : 1; /** Set -> user writable. (read/write bit) */
            uint32_t user : 1;     /** Set -> user accessible. */
            uint32_t unused0 : 2;  /** Unused 2 caching bits. */
            uint32_t accessed : 1; /** Set -> accessed sinced mapped. */
            uint32_t unused1 : 1;  /** Unused bit. */
            uint32_t size : 1;     /** 0 -> using 4KiB page size. */
        } __attribute__((packed)) flags_bitmap;
        uint8_t flags_byte;
    } flags;

    uint32_t unused2 : 4; /** Unused 4 misc bits. */
    uint32_t frame : 20;  /** Physical frame number of level-2 table. */
} __attribute__((packed)) page_directory_entry;

extern page_directory_entry *page_directory __attribute__((aligned(4096)));
extern page_table_entry *page_table __attribute__((aligned(4096)));

// Walk the page directory and return the physical address mapped at virtAddr,
// or NULL if not mapped. Useful for debugging and for copy-on-write later.
void *paging_get_physical(void *virtAddr);

// return the virtual address mapped at physAddr,
// or NULL if not mapped.
void *paging_get_virtual(void *physAddr);

static inline uint32_t phys_to_page_index(void *physAddr);

static inline void *page_index_to_phys(uint32_t index);

int paging_check_page(uint32_t page_index);
void paging_mark_page_used(uint32_t page_index);
void paging_mark_page_free(uint32_t page_index);
int paging_find_free_page();

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
void paging_map_page(void *virtAddr, void *physAddr, uint32_t flags);

// Unmap a single virtual page (does NOT free the underlying frame).
void paging_unmap_page(void *virtAddr);

// Allocate a free frame AND map it at virtAddr in one step.
// Returns the physical address on success, NULL on OOM.
void *paging_alloc_and_map(void *virtAddr, uint32_t flags);

// Map a contiguous physical region [physAddr, physAddr + size) to
// [virtAddr, virtAddr + size). Rounds up to page boundaries.
// Used for MMIO regions and framebuffers where the physical address is fixed.
void paging_map_region(void *virtAddr, void *physAddr, size_t size, uint32_t flags);

// Unmap [virtAddr, virtAddr + size) and free the backing frames.
void paging_free_region(void *virtAddr, size_t size);

void paging_init(void *page_table);
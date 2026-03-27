/*
 * File: paging.h
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>

#define PDE_PRESENT     0x1
#define PDE_RW          0x2
#define PDE_USER        0x4
#define PAGE_HUGE_PAGES         0x0

#define PT_ADDR_MASK    0xffffffffff000

/* CR3 register value — physical address of PML4 + flags */
typedef union {
    uint64_t raw;
    struct {
        uint64_t ignored0   : 3;
        uint64_t pwt        : 1;   /* Page-level write-through */
        uint64_t pcd        : 1;   /* Page-level cache disable */
        uint64_t ignored1   : 7;
        uint64_t pml4_pfn   : 40;  /* Physical address >> 12 */
        uint64_t reserved   : 12;
    } __attribute__((packed));
} cr3_t;

/* One entry in PML4, PDPT, PD, or PT — they share this layout */
typedef union {
    uint64_t raw;
    struct {
        uint64_t present    : 1;   /* Must be 1 to be valid */
        uint64_t rw         : 1;   /* 0 = read-only, 1 = read/write */
        uint64_t us         : 1;   /* 0 = supervisor, 1 = user */
        uint64_t pwt        : 1;   /* Page-level write-through */
        uint64_t pcd        : 1;   /* Page-level cache disable */
        uint64_t accessed   : 1;   /* Set by CPU on read */
        uint64_t dirty      : 1;   /* Set by CPU on write (Page Table only) */
        uint64_t ps_or_pat  : 1;   /* PS in Page Table/Page Dir Ptr Table = huge page; PAT in Page Table */
        uint64_t global     : 1;   /* Don't flush on CR3 reload (Page Table only) */
        uint64_t avail0     : 3;   /* Free for OS use */
        uint64_t pfn        : 40;  /* Physical address of next table >> 12 */
        uint64_t avail1     : 11;  /* Free for OS use */
        uint64_t nx         : 1;   /* No-execute (requires EFER.NXE) */
    } __attribute__((packed));
} pte64_t;

/* A full table is 512 entries, filling exactly one 4 KiB frame */
#define PT64_ENTRIES 512

typedef struct { pte64_t e[PT64_ENTRIES]; } __attribute__((aligned(4096))) pml4_t;
typedef struct { pte64_t e[PT64_ENTRIES]; } __attribute__((aligned(4096))) pdpt_t;
typedef struct { pte64_t e[PT64_ENTRIES]; } __attribute__((aligned(4096))) pd_t;
typedef struct { pte64_t e[PT64_ENTRIES]; } __attribute__((aligned(4096))) pt_t;

extern pml4_t *page_map_level_4_table;
extern pdpt_t *page_directory_pointer_table;
extern pd_t *page_directory_table;
extern pdpt_t *page_directory_pointer_table_kernel;
extern pd_t *page_directory_table_kernel;
extern cr3_t *_cr3;

void fill_32bit_table();
void fill_64bit_table();
/*
 * File: paging_64.h
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include "kernel/mmu_types.h"
#include "debug/debug.h"
#include <util/binary.h>

#ifdef __x86_64__
#define PAGING_64 1
#define GET_PAGE_OFF(va) (((uint64_t)va) & 0xFFF)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ull
#define PAGE_FLAGS_MASK 0xFFF0000000000FFFull

/* A full table is 512 entries, filling exactly one 4 KiB frame */
#define PT64_ENTRIES 512
#define PAGE_TABLE_ENTRIES 512

#define PAGE_PRESENT_BIT 0
#define PAGE_PRESENT BIT(PAGE_PRESENT_BIT)       /* 0 = disabled, 1 = enabled */
#define PAGE_WRITABLE_BIT 1
#define PAGE_WRITABLE BIT(PAGE_WRITABLE_BIT)     /* 0 = read-only, 1 = read/write */
#define PAGE_USER_BIT 2
#define PAGE_USER BIT(PAGE_USER_BIT)             /* 0 = supervisor, 1 = user */
#define PAGE_PWT_BIT 3
#define PAGE_PWT BIT(PAGE_PWT_BIT)               /* Page-level write-through */
#define PAGE_PCD_BIT 4
#define PAGE_PCD BIT(PAGE_PCD_BIT)               /* Page-level cache disable */
#define PAGE_ACCESSED_BIT 5
#define PAGE_ACCESSED BIT(PAGE_ACCESSED_BIT)     /* Set by CPU on read */
#define PAGE_DIRTY_BIT 6
#define PAGE_DIRTY BIT(PAGE_DIRTY_BIT)           /* Set by CPU on write page table only */
#define PAGE_AVAIL0_BIT 6
#define PAGE_AVAIL0 BIT(PAGE_AVAIL0_BIT)         /* Free for OS use pde/pdpte/pml4e/pml5e/huge only */
#define PAGE_HUGE_PAGES_BIT 7
#define PAGE_HUGE_PAGES BIT(PAGE_HUGE_PAGES_BIT) /* Must be 1 for 2 MiB page */
#define PAGE_PAT_BIT 7
#define PAGE_PAT BIT(PAGE_PAT_BIT)               /* PAT in Page Table */
#define PAGE_GLOBAL_BIT 8
#define PAGE_GLOBAL BIT(PAGE_GLOBAL_BIT)         /* Don't flush on CR3 reload huge pages and page table only */
#define PAGE_COW_BIT 9
#define PAGE_COW BIT(PAGE_COW_BIT)               /* 0 = Not using CoW, 1 = Using CoW*/
#define PAGE_NO_EXEC_BIT 63
#define PAGE_NO_EXEC BIT(PAGE_NO_EXEC_BIT)

// 0b01100101

typedef uint64_t paging_flags;

/* CR3 register value — physical address of PML4 + flags */
typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t ignored0 : 3;
        uint64_t pwt : 1;        /* Page-level write-through */
        uint64_t pcd : 1;        /* Page-level cache disable */
        uint64_t ignored1 : 7;
        uint64_t pml4_addr : 40; /* Physical address >> 12 */
        uint64_t reserved : 12;
    } __attribute__((packed));
} cr3_t;

/* 2 MiB Page Directory entry (PS=1) */
typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t present : 1;   /* Must be 1 to be valid */
        uint64_t writable : 1;  /* 0 = read-only, 1 = read/write */
        uint64_t user : 1;      /* 0 = supervisor, 1 = user */
        uint64_t pwt : 1;       /* Page-level write-through */
        uint64_t pcd : 1;       /* Page-level cache disable */
        uint64_t accessed : 1;  /* Set by CPU on read */
        uint64_t dirty : 1;     /* Set by CPU on write */
        uint64_t ps : 1;        /* Must be 1 for 2 MiB page */
        uint64_t global : 1;    /* Don't flush on CR3 reload */
        uint64_t cow : 1;       /* 0 = Not using CoW, 1 = Using CoW */
        uint64_t avail0 : 2;    /* Free for OS use */
        uint64_t pat : 1;       /* PAT index bit 2 (bit 12) */
        uint64_t reserved0 : 8; /* Must be 0 (bits 13-20) */
        uint64_t addr : 31;     /* Physical address of 2 MiB page >> 21 */
        uint64_t avail1 : 7;    /* Free for OS use (bits 52-58) */
        uint64_t pke : 4;       /* Protection key (requires CR4.PKE) */
        uint64_t nx : 1;        /* No-execute (requires EFER.NXE) */
    } __attribute__((packed));
} pd_huge_entry64;              /* 2 MiB */

/* 1 GiB Page Directory Pointer entry (PS=1) */
typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t present : 1;    /* Must be 1 to be valid */
        uint64_t writable : 1;   /* 0 = read-only, 1 = read/write */
        uint64_t user : 1;       /* 0 = supervisor, 1 = user */
        uint64_t pwt : 1;        /* Page-level write-through */
        uint64_t pcd : 1;        /* Page-level cache disable */
        uint64_t accessed : 1;   /* Set by CPU on read */
        uint64_t dirty : 1;      /* Set by CPU on write */
        uint64_t ps : 1;         /* Must be 1 for 1 GiB page */
        uint64_t global : 1;     /* Don't flush on CR3 reload */
        uint64_t cow : 1;        /* 0 = Not using CoW, 1 = Using CoW */
        uint64_t avail0 : 2;     /* Free for OS use */
        uint64_t pat : 1;        /* PAT index bit 2 (bit 12) */
        uint64_t reserved0 : 17; /* Must be 0 (bits 13-29) */
        uint64_t addr : 22;      /* Physical address of 1 GiB page >> 30 */
        uint64_t avail1 : 7;     /* Free for OS use (bits 52-58) */
        uint64_t pke : 4;        /* Protection key (requires CR4.PKE) */
        uint64_t nx : 1;         /* No-execute (requires EFER.NXE) */
    } __attribute__((packed));
} pdpt_huge_entry64;             /* 1 GiB */

typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t present : 1;   /* Must be 1 to be valid */
        uint64_t writable : 1;  /* 0 = read-only, 1 = read/write */
        uint64_t user : 1;      /* 0 = supervisor, 1 = user */
        uint64_t pwt : 1;       /* Page-level write-through */
        uint64_t pcd : 1;       /* Page-level cache disable */
        uint64_t accessed : 1;  /* Set by CPU on read */
        uint64_t dirty : 1;     /* Set by CPU on write (Page Table only) */
        uint64_t ps_or_pat : 1; /* PS in Page Table/Page Dir Ptr Table = huge page; PAT in Page Table */
        uint64_t global : 1;    /* Don't flush on CR3 reload (Page Table only) */
        uint64_t cow : 1;       /* 0 = Not using CoW, 1 = Using CoW */
        uint64_t avail0 : 2;    /* Free for OS use */
        uint64_t addr : 40;     /* Physical address of next table >> 12 */
        uint64_t avail1 : 11;   /* Free for OS use */
        uint64_t nx : 1;        /* No-execute (requires EFER.NXE) */
    } __attribute__((packed));
} page_table_entry64;

typedef struct
{
    page_table_entry64 e[PT64_ENTRIES];
} __attribute__((aligned(4096))) page_map_level_4;

typedef struct
{
    page_table_entry64 e[PT64_ENTRIES];
} __attribute__((aligned(4096))) page_dpt;

typedef struct
{
    page_table_entry64 e[PT64_ENTRIES];
} __attribute__((aligned(4096))) page_directory64;

typedef struct
{
    page_table_entry64 e[PT64_ENTRIES];
} __attribute__((aligned(4096))) page_table64;

typedef enum
{
    PAGING_LEVEL_PML4 = 4,
    PAGING_LEVEL_PDPT = 3,
    PAGING_LEVEL_PD = 2,
    PAGING_LEVEL_PT = 1,
} paging_level;

void *paging64_get_table(page_table_t *page_table, vaddr_t virtAddr, paging_level target_level, int alloc, bool user);
page_table_entry64 *paging64_get_entry(page_table_t *page_table, vaddr_t virtAddr, paging_level target_level, bool alloc, bool user);

page_map_level_4 *paging64_get_pml4(page_table_t *p, vaddr_t v, int alloc, bool user);
page_dpt *paging64_get_pdpt(page_table_t *p, vaddr_t v, int alloc, bool user);
page_directory64 *paging64_get_pd(page_table_t *p, vaddr_t v, int alloc, bool user);
page_table64 *paging64_get_pt(page_table_t *p, vaddr_t v, int alloc, bool user);

page_table_entry64 *paging64_get_pml4_entry(page_table_t *p, vaddr_t v, int alloc, bool user);
page_table_entry64 *paging64_get_pdpt_entry(page_table_t *p, vaddr_t v, int alloc, bool user);
page_table_entry64 *paging64_get_pd_entry(page_table_t *p, vaddr_t v, int alloc, bool user);
page_table_entry64 *paging64_get_pt_entry(page_table_t *p, vaddr_t v, int alloc, bool user);

paging_flags mmu_flags_to_pte(mmu_flags_t flags);
mmu_flags_t pte_to_mmu_flags(paging_flags pte);
void paging_init64(page_table_t *kernel_page, vaddr_t kernel_addr, size_t kernel_size, paddr_t pmm_start, paddr_t pmm_end);
#endif

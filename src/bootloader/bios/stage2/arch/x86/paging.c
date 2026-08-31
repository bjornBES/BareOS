/*
 * File: paging.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"

#include "memory.h"

#include <defs.h>

void paging32_init(boot_params_t *bp)
{
}

#ifdef __x86_64__

/* CR3 register value — physical address of PML4 + flags */
typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t ignored0 : 3;
        uint64_t pwt : 1;       /* Page-level write-through */
        uint64_t pcd : 1;       /* Page-level cache disable */
        uint64_t ignored1 : 7;
        uint64_t pml4_pfn : 40; /* Physical address >> 12 */
        uint64_t reserved : 12;
    } PACKED;
} cr3_t;

/* One entry in PML4, PDPT, PD, or PT — they share this layout */
typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t present : 1;   /* Must be 1 to be valid */
        uint64_t rw : 1;        /* 0 = read-only, 1 = read/write */
        uint64_t us : 1;        /* 0 = supervisor, 1 = user */
        uint64_t pwt : 1;       /* Page-level write-through */
        uint64_t pcd : 1;       /* Page-level cache disable */
        uint64_t accessed : 1;  /* Set by CPU on read */
        uint64_t dirty : 1;     /* Set by CPU on write (Page Table only) */
        uint64_t ps_or_pat : 1; /* PS in Page Table/Page Dir Ptr Table = huge page; PAT in Page Table */
        uint64_t global : 1;    /* Don't flush on CR3 reload (Page Table only) */
        uint64_t avail0 : 3;    /* Free for OS use */
        uint64_t pfn : 40;      /* Physical address of next table >> 12 */
        uint64_t avail1 : 11;   /* Free for OS use */
        uint64_t nx : 1;        /* No-execute (requires EFER.NXE) */
    } PACKED;
} pte64_t;

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
        uint64_t avail0 : 3;    /* Free for OS use */
        uint64_t pat : 1;       /* PAT index bit 2 (bit 12) */
        uint64_t reserved0 : 8; /* Must be 0 (bits 13-20) */
        uint64_t addr : 31;     /* Physical address of 2 MiB page >> 21 */
        uint64_t avail1 : 7;    /* Free for OS use (bits 52-58) */
        uint64_t pke : 4;       /* Protection key (requires CR4.PKE) */
        uint64_t nx : 1;        /* No-execute (requires EFER.NXE) */
    } PACKED;
} pd_huge_entry64;              /* 2 MiB */

/* A full table is 512 entries, filling exactly one 4 KiB frame */
#define PT64_ENTRIES 512

typedef struct
{
    pte64_t e[PT64_ENTRIES];
} ALIGN(4096) pml4_t;

typedef struct
{
    pte64_t e[PT64_ENTRIES];
} ALIGN(4096) pdpt_t;

typedef struct
{
    pte64_t e[PT64_ENTRIES];
} ALIGN(4096) pd_t;

typedef struct
{
    pte64_t e[PT64_ENTRIES];
} ALIGN(4096) pt_t;

pml4_t pml4_table ALIGN(4096);
pdpt_t pdpt_table_bootloader ALIGN(4096);
pd_t pd_table_bootloader ALIGN(4096);
pt_t pt_table_bootloader ALIGN(4096);
pdpt_t pdpt_table_kernel ALIGN(4096);
pd_t pd_table_kernel ALIGN(4096);
cr3_t _cr3 ALIGN(4096);

#define VA_PML4_IDX(va) (((va) >> 39) & 0x1FF)
#define VA_PDPT_IDX(va) (((va) >> 30) & 0x1FF)
#define VA_PD_IDX(va)   (((va) >> 21) & 0x1FF)
#define VA_PT_IDX(va)   (((va) >> 12) & 0x1FF)
#define VA_PAGE_OFF(va) ((va) & 0xFFF)

void paging64_init(boot_params_t *bp)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    memset(&pml4_table, 0, sizeof(pml4_table));
    memset(&pdpt_table_bootloader, 0, sizeof(pdpt_table_bootloader));
    memset(&pd_table_bootloader, 0, sizeof(pd_table_bootloader));
    memset(&pt_table_bootloader, 0, sizeof(pt_table_bootloader));
    memset(&pdpt_table_kernel, 0, sizeof(pdpt_table_kernel));
    memset(&pd_table_kernel, 0, sizeof(pd_table_kernel));
    memset(&_cr3, 0, sizeof(_cr3));

    pml4_table.e[0].raw = 0;
    pml4_table.e[0].present = 1;
    pml4_table.e[0].rw = 1;
    pml4_table.e[0].pfn = (uint64_t)(&pdpt_table_bootloader) >> 12;

    pdpt_table_bootloader.e[0].raw = 0;
    pdpt_table_bootloader.e[0].present = 1;
    pdpt_table_bootloader.e[0].rw = 1;
    pdpt_table_bootloader.e[0].pfn = (uint64_t)(&pd_table_bootloader) >> 12;

    pd_table_bootloader.e[0].raw = 0;
    pd_table_bootloader.e[0].present = 1;
    pd_table_bootloader.e[0].rw = 1;
    pd_table_bootloader.e[0].pfn = (uint64_t)(&pt_table_bootloader) >> 12;

    for (int i = 0; i < PT64_ENTRIES; i++)
    {
        pt_table_bootloader.e[i].raw = 0;
        pt_table_bootloader.e[i].present = 1;
        pt_table_bootloader.e[i].rw = 1;
        pt_table_bootloader.e[i].pfn = i;
    }

    int pml4_kernel_index = VA_PML4_IDX(bp->kernel_virt_base);
    pml4_table.e[pml4_kernel_index].raw = 0;
    pml4_table.e[pml4_kernel_index].pfn = ((uint64_t)&pdpt_table_kernel) >> 12;
    pml4_table.e[pml4_kernel_index].present = 1;
    pml4_table.e[pml4_kernel_index].rw = 1;

    // pdpt mapping ? -> ?
    int pdpt_kernel_index = VA_PDPT_IDX(bp->kernel_virt_base);
    pdpt_table_kernel.e[pdpt_kernel_index].raw = 0;
    pdpt_table_kernel.e[pdpt_kernel_index].pfn = ((uint64_t)&pd_table_kernel) >> 12;
    pdpt_table_kernel.e[pdpt_kernel_index].present = 1;
    pdpt_table_kernel.e[pdpt_kernel_index].rw = 1;

    // pd mapping ? -> ?
    int pd_kernel_index = VA_PD_IDX(bp->kernel_virt_base);
    pd_huge_entry64 *entries = (pd_huge_entry64 *)pd_table_kernel.e;
    entries[pd_kernel_index].raw = 0;
    entries[pd_kernel_index].present = 1;
    entries[pd_kernel_index].writable = 1;
    entries[pd_kernel_index].ps = 1;
    entries[pd_kernel_index].addr = 1;

    entries[pd_kernel_index + 1].raw = 0;
    entries[pd_kernel_index + 1].present = 1;
    entries[pd_kernel_index + 1].writable = 1;
    entries[pd_kernel_index + 1].ps = 1;
    entries[pd_kernel_index + 1].addr = 2;

    entries[pd_kernel_index + 2].raw = 0;
    entries[pd_kernel_index + 2].present = 1;
    entries[pd_kernel_index + 2].writable = 1;
    entries[pd_kernel_index + 2].ps = 1;
    entries[pd_kernel_index + 2].addr = 3;

    // uint32_t cr3 = (uint64_t)&pml4_table >> 12;
    _cr3.pml4_pfn = ((uint64_t)&pml4_table) >> 12;

    // __asm__("mov cr3,%0" : : "r"(cr3));
#pragma GCC diagnostic pop
}

#endif

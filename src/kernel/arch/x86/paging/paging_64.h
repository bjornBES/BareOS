/*
 * File: paging_64.h
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

/* A full table is 512 entries, filling exactly one 4 KiB frame */
#define PT64_ENTRIES 512

typedef uint64_t paging_flags;

/* CR3 register value — physical address of PML4 + flags */
typedef union
{
    uint64_t raw;
    struct
    {
        uint64_t ignored0 : 3;
        uint64_t pwt : 1; /* Page-level write-through */
        uint64_t pcd : 1; /* Page-level cache disable */
        uint64_t ignored1 : 7;
        uint64_t pml4_addr : 40; /* Physical address >> 12 */
        uint64_t reserved : 12;
    } __attribute__((packed));
} cr3_t;

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
        uint64_t avail0 : 3;    /* Free for OS use */
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

extern cr3_t kernel_cr3;

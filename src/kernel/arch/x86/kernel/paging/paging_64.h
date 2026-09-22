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
#include <types.h>
#include <binary.h>
#include "mm/mmu.h"

#ifdef __x86_64__
#define GET_PAGE_OFF(va) (((uint64_t)va) & 0xFFF)
#define PAGE_ADDR_MASK   0x000FFFFFFFFFF000ull
#define PAGE_FLAGS_MASK  0xFFF0000000000FFFull

/* A full table is 512 entries, filling exactly one 4 KiB frame */
#define PT64_ENTRIES        512
#define PAGE_TABLE_ENTRIES  512

#define PAGE_PRESENT_BIT    0
#define PAGE_PRESENT        BIT(PAGE_PRESENT_BIT)    /* 0 = disabled, 1 = enabled */
#define PAGE_WRITABLE_BIT   1
#define PAGE_WRITABLE       BIT(PAGE_WRITABLE_BIT)   /* 0 = read-only, 1 = read/write */
#define PAGE_USER_BIT       2
#define PAGE_USER           BIT(PAGE_USER_BIT)       /* 0 = supervisor, 1 = user */
#define PAGE_PWT_BIT        3
#define PAGE_PWT            BIT(PAGE_PWT_BIT)        /* Page-level write-through */
#define PAGE_PCD_BIT        4
#define PAGE_PCD            BIT(PAGE_PCD_BIT)        /* Page-level cache disable */
#define PAGE_ACCESSED_BIT   5
#define PAGE_ACCESSED       BIT(PAGE_ACCESSED_BIT)   /* Set by CPU on read */
#define PAGE_DIRTY_BIT      6
#define PAGE_DIRTY          BIT(PAGE_DIRTY_BIT)      /* Set by CPU on write page table only */
#define PAGE_HUGE_PAGES_BIT 7
#define PAGE_HUGE_PAGES     BIT(PAGE_HUGE_PAGES_BIT) /* Must be 1 for 2 MiB page */
#define PAGE_PAT_BIT        7
#define PAGE_PAT            BIT(PAGE_PAT_BIT)        /* PAT in Page Table */
#define PAGE_GLOBAL_BIT     8
#define PAGE_GLOBAL         BIT(PAGE_GLOBAL_BIT)     /* Don't flush on CR3 reload PS and PT only */
#define PAGE_COW_BIT        9
#define PAGE_COW            BIT(PAGE_COW_BIT)        /* 0 = Not using CoW, 1 = Using CoW*/
#define PAGE_NO_EXEC_BIT    63
#define PAGE_NO_EXEC        BIT(PAGE_NO_EXEC_BIT)

// 0b01100101

typedef uint64_t paging_flags;

/* CR3 register value — physical address of PML4 + flags */
typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t ignored0 : 3;
        uint64_t pwt : 1;   /* Page-level write-through */
        uint64_t pcd : 1;   /* Page-level cache disable */
        uint64_t ignored1 : 7;
        uint64_t addr : 40; /* Physical address >> 12 */
        uint64_t reserved : 12;
    } PACKED;
} cr3_t;

/* 2 MiB Page Directory entry (PS=1) */
typedef union
{
    uint64_t raw;

    struct
    {
        /// @brief Indicates the entry is present and readable
        /// @note Must be 1 to be valid
        uint64_t present : 1;

        /// @brief If 1 indicates the entry is writeable, if 0 the entry is read-only
        /// @note 0 = read-only, 1 = read/write
        uint64_t writable : 1;

        /// @brief If 1 indicates the entry is a user page, if 0 the entry is supervisor only
        /// @note 0 = supervisor, 1 = user
        uint64_t user : 1;

        /// @brief If 1 indicates the write should skip the cache and immediately write to memory.
        /// @note Page write through. PWT=1 write immediately to memory, PWT=0 write data to the cache.
        /// Is bit 0 in the PAT index.
        uint64_t pwt : 1;

        /// @brief If 1 indicates the entries data can't be cached
        /// @note Page-level cache disable.
        /// Is bit 1 in the PAT index
        uint64_t pcd : 1;

        /// @brief If 1 indicates the entry has been read by the CPU
        /// @note Set by CPU on read
        uint64_t accessed : 1;

        /// @brief If 1 indicates the entry has been written to by the CPU
        /// @note Set by CPU on write only for leaf entries only
        uint64_t dirty : 1;

        /// @brief If 1 indicates the entry size of 2 MiB.
        uint64_t ps : 1;

        /// @brief If 1 indicates the entry can't be flushed on a CR3 reload
        /// @note only for leaf entries
        uint64_t global : 1;

        /// @brief If 1 indicates the entry is allocated with cow
        /// @pre writable = 0 && present = 1
        /// @note 0 = Not using CoW, 1 = Using CoW only for leaf entries
        uint64_t cow : 1;

        /// @brief If 1 indicates the entry is about to be destroyed
        /// @note 1 = The entry needs to be unmapped only for leaf entries
        uint64_t destroy : 1;

        /// @brief Free for OS use
        uint64_t avail0 : 1;

        /// @brief Is apart of the PAT index
        /// @note is the bit 2 in the PAT index
        uint64_t pat : 1;

        /// @brief Must be 0 (bits 13-20)
        /// @note If non zero a #PF will happen with bit 3 set
        uint64_t reserved0 : 8;

        /// @brief Indicates the physical frame index that this page entry is pointing to
        /// @note Physical address of 2 MiB the frame >> 21
        uint64_t addr : 31;

        /// @brief Free for OS use
        uint64_t avail1 : 7;

        /// @brief Indicates the protection key of the given entry
        /// @pre PKU (CPUID.0x07.0x00:ECX[3]) or PKS (CPUID.0x07.0x00:ECX[31]) and CR4.PKE
        /// @note only for leaf entries
        uint64_t pke : 4;

        /// @brief Indicates the entry isn't executable
        /// @pre EXECUTE_DIS (CPUID.0x80000001:EDX[20]) and EFER.NXE
        /// @note only for leaf entries
        uint64_t nx : 1;
    } PACKED;
} pd_huge_entry64; /* 2 MiB */

/* 1 GiB Page Directory Pointer entry (PS=1) */
typedef union
{
    uint64_t raw;

    struct
    {
        /// @brief Indicates the entry is present and readable
        /// @note Must be 1 to be valid
        uint64_t present : 1;

        /// @brief If 1 indicates the entry is writeable, if 0 the entry is read-only
        /// @note 0 = read-only, 1 = read/write
        uint64_t writable : 1;

        /// @brief If 1 indicates the entry is a user page, if 0 the entry is supervisor only
        /// @note 0 = supervisor, 1 = user
        uint64_t user : 1;

        /// @brief If 1 indicates the write should skip the cache and immediately write to memory.
        /// @note Page write through. PWT=1 write immediately to memory, PWT=0 write data to the cache.
        /// Is bit 0 in the PAT index.
        uint64_t pwt : 1;

        /// @brief If 1 indicates the entries data can't be cached
        /// @note Page-level cache disable.
        /// Is bit 1 in the PAT index
        uint64_t pcd : 1;

        /// @brief If 1 indicates the entry has been read by the CPU
        /// @note Set by CPU on read
        uint64_t accessed : 1;

        /// @brief If 1 indicates the entry has been written to by the CPU
        /// @note Set by CPU on write only for leaf entries only
        uint64_t dirty : 1;

        /// @brief If 1 indicates the entry size of 1 GiB.
        uint64_t ps : 1;

        /// @brief If 1 indicates the entry can't be flushed on a CR3 reload
        /// @note only for leaf entries
        uint64_t global : 1;

        /// @brief If 1 indicates the entry is allocated with cow
        /// @pre writable = 0 && present = 1
        /// @note 0 = Not using CoW, 1 = Using CoW only for leaf entries
        uint64_t cow : 1;

        /// @brief If 1 indicates the entry is about to be destroyed
        /// @note 1 = The entry needs to be unmapped only for leaf entries
        uint64_t destroy : 1;

        /// @brief Free for OS use
        uint64_t avail0 : 1;

        /// @brief Is apart of the PAT index
        /// @note is the bit 2 in the PAT index
        uint64_t pat : 1;

        /// @brief Must be 0 (bits 13-29)
        /// @note If non zero a #PF will happen with bit 3 set
        uint64_t reserved0 : 17;

        /// @brief Indicates the physical frame index that this page entry is pointing to
        /// @note Physical address of 1 GiB the frame >> 30
        uint64_t addr : 22;

        /// @brief Free for OS use
        uint64_t avail1 : 7;

        /// @brief Indicates the protection key of the given entry
        /// @pre PKU (CPUID.0x07.0x00:ECX[3]) or PKS (CPUID.0x07.0x00:ECX[31]) and CR4.PKE
        /// @note only for leaf entries
        uint64_t pke : 4;

        /// @brief Indicates the entry isn't executable
        /// @pre EXECUTE_DIS (CPUID.0x80000001:EDX[20]) and EFER.NXE
        /// @note only for leaf entries
        uint64_t nx : 1;
    } PACKED;
} pdpt_huge_entry64; /* 1 GiB */

typedef union
{
    uint64_t raw;

    struct
    {
        /// @brief Indicates the entry is present and readable
        /// @note Must be 1 to be valid
        uint64_t present : 1;

        /// @brief If 1 indicates the entry is writeable, if 0 the entry is read-only
        /// @note 0 = read-only, 1 = read/write
        uint64_t writable : 1;

        /// @brief If 1 indicates the entry is a user page, if 0 the entry is supervisor only
        /// @note 0 = supervisor, 1 = user
        uint64_t user : 1;

        /// @brief If 1 indicates the write should skip the cache and immediately write to memory.
        /// @note Page write through. PWT=1 write immediately to memory, PWT=0 write data to the cache.
        /// Is bit 0 in the PAT index.
        uint64_t pwt : 1;

        /// @brief If 1 indicates the entries data can't be cached
        /// @note Page-level cache disable.
        /// Is bit 1 in the PAT index
        uint64_t pcd : 1;

        /// @brief If 1 indicates the entry has been read by the CPU
        /// @note Set by CPU on read
        uint64_t accessed : 1;

        /// @brief If 1 indicates the entry has been written to by the CPU
        /// @note Set by CPU on write only for leaf entries only
        uint64_t dirty : 1;

        /// @brief Is apart of the PAT index
        /// @note is the bit 2 in the PAT index
        uint64_t pat : 1;

        /// @brief If 1 indicates the entry can't be flushed on a CR3 reload
        /// @note only for leaf entries
        uint64_t global : 1;

        /// @brief If 1 indicates the entry is allocated with cow
        /// @pre writable = 0 && present = 1
        /// @note 0 = Not using CoW, 1 = Using CoW only for leaf entries
        uint64_t cow : 1;

        /// @brief If 1 indicates the entry is about to be destroyed
        /// @note 1 = The entry needs to be unmapped only for leaf entries
        uint64_t destroy : 1;

        /// @brief Free for OS use
        uint64_t avail0 : 1;

        /// @brief Indicates the physical frame index that this page entry is pointing to
        /// @note Physical address of the frame >> 12
        uint64_t addr : 40;

        /// @brief Free for OS use
        uint64_t avail1 : 7;

        /// @brief Indicates the protection key of the given entry
        /// @pre PKU (CPUID.0x07.0x00:ECX[3]) or PKS (CPUID.0x07.0x00:ECX[31]) and CR4.PKE
        /// @note only for leaf entries
        uint64_t pke : 4;

        /// @brief Indicates the entry isn't executable
        /// @pre EXECUTE_DIS (CPUID.0x80000001:EDX[20]) and EFER.NXE
        /// @note only for leaf entries
        uint64_t nx : 1;
    } PACKED;
} page_table_entry64_leaf;

typedef union
{
    uint64_t raw;

    struct
    {
        /// @brief Indicates the entry is present and readable
        /// @note Must be 1 to be valid
        uint64_t present : 1;

        /// @brief If 1 indicates the entry is writeable, if 0 the entry is read-only
        /// @note 0 = read-only, 1 = read/write
        uint64_t writable : 1;

        /// @brief If 1 indicates the entry is a user page, if 0 the entry is supervisor only
        /// @note 0 = supervisor, 1 = user
        uint64_t user : 1;

        /// @brief If a table changes: If 1 indicates write should skip the cache and immediately write to memory.
        /// @pre pcd=0
        /// @note if an entry within the table changes it should: PWT=1 write the changes immediately to memory, PWT=0 write the changes to the cache first.
        uint64_t pwt : 1;

        /// @brief If 1 indicates the next table shouldn't be cached in the cache lines.
        /// @note PCD=1 don't cache the next table, PCD=0 the CPU can cache the next table.
        uint64_t pcd : 1;

        /// @brief If 1 indicates the entry has been read by the CPU
        /// @note Set by CPU on read
        uint64_t accessed : 1;

        /// @brief Free for OS use
        uint64_t avail0 : 1;

        /// @brief If PD/PDPT indicates the entry is huge/large, PML4: Reserved must be 0
        uint64_t ps : 1;

        /// @brief Free for OS use
        uint64_t avail1 : 4;

        /// @brief Indicates the physical frame index that this page entry is pointing to
        /// @note Physical address of next table >> 12
        uint64_t addr : 40;

        /// @brief Free for OS use
        uint64_t avail2 : 11;

        /// @brief Must be 0 on non leaf entries
        /// @note It's a shared switch that would clobber all sibling mappings if you touched it, so DONT TOUCHED IS. ~ok~
        uint64_t reserved : 1;
    } PACKED;
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
    page_table_entry64_leaf e[PT64_ENTRIES];
} __attribute__((aligned(4096))) page_table64;

paging_flags mmu_flags_to_pte(mmu_flags_t flags);
mmu_flags_t pte_to_mmu_flags(paging_flags pte);

#endif

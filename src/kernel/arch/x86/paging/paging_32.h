/*
 * File: paging.h
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Mar 2026
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

#ifdef __i686__

#define PAGE_SIZE 4096

#define GETPAGEDIRECTORYINDEX(addr) (uint32_t)addr >> 22
#define GETPAGETABLEINDEX(addr) (uint32_t)addr >> 12 & 0x3FF

typedef uint32_t paging_flags;

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
                uint32_t unused2 : 4;  /** Unused bits. */
            } flags_bitmap;
            uint8_t flags_byte;
        } flags;
        uint32_t frame : 20; /** Physical frame number of the page. */
    };
    uint32_t raw;
} __attribute__((packed)) page_table_entry;
typedef struct
{
    page_table_entry entries[1024];
} page_table32;
typedef union
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
        uint32_t unused2 : 4;  /** Unused bits. */
        uint32_t frame : 20;   /** Physical frame number of level-2 table. */
    };
    uint32_t raw;
} __attribute__((packed)) page_directory_entry32;
typedef struct page_directory_t
{
    page_directory_entry32 entries[1024];
} page_directory32;

extern page_directory32 *kernel_page_directory;

#endif

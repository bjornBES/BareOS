/*
 * File: buddy.c
 * File Created: 16 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "buddy.h"
#include "pmm.h"

#include "memory/paging/paging.h"

#define MODULE "BUDDY"

#define BUDDY_BLOCK_SIZE(order) (uint32_64)((1 << order) * PAGE_SIZE)
#define MAX_ORDER0_BLOCKS (0x40000000 / PAGE_SIZE)

typedef struct
{
    phys_addr *blocks;
    size_t count;
} buddy_order;

typedef struct
{
    buddy_order orders[BUDDY_MAX_ORDER];
} buddy;

buddy global_buddy;
phys_addr start;
phys_addr end;

void buddy_init(boot_params *params)
{
    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++)
    {
        size_t max_blocks = MAX_ORDER0_BLOCKS >> i;
        size_t bytes = max_blocks * sizeof(phys_addr);
        size_t num_frames = ALIGN_2_UP(bytes, PAGE_SIZE) / PAGE_SIZE;

        phys_addr phys = paging_get_kernel_frame();
        for (size_t f = 1; f < num_frames; f++)
        {
            paging_get_kernel_frame(); // advance the bump index
        }

        virt_addr virt = phys_to_virt_auto(phys);
        log_debug(MODULE, "frame phys=%p virt=%p", phys, virt);
        global_buddy.orders[i].blocks = virt;
        global_buddy.orders[i].count = 0;
    }

    E820_entry *entries = params->e820Entries;
    log_debug(MODULE, "count = %u", params->e820Count);
    for (size_t i = 0; i < params->e820Count; i++)
    {
        E820_entry *entry = &entries[i];
        log_debug(MODULE, "%i: %u %p-%p", i, entry->type, entry->addr, entry->addr + entry->size);
        if (entry->type != MEMORY_AVAILABLE)
        {
            continue;
        }

        start = (phys_addr)(entry->addr);
        end = (phys_addr)(entry->addr + entry->size);

        if (end <= (phys_addr)0x100000)
        {
            continue;
        }
        if (start < (phys_addr)0x100000)
        {
            start = (phys_addr)0x100000;
        }

        if (start < KERNEL_PHYS_END)
        {
            log_debug(MODULE, "region start=%p end=%p", start, end);
            start = (phys_addr)ALIGN_2_UP((uint32_64)KERNEL_PHYS_END, PAGE_SIZE);
        }
        start = (phys_addr)ALIGN_2_UP((uint64_t)start, 0x200000);
        end = (phys_addr)ALIGN_2_DOWN((uint64_t)end, 0x1000000);
        log_debug(MODULE, "region start=%p end=%p", start, end);
    }
}

void buddy_map()
{
    log_debug(MODULE, "mapping the frames");
    log_debug(MODULE, "%p..%p size %x", start, end, end - start);
    phys_addr addr = start;
    while (addr < end)
    {
        uint32_t order = BUDDY_MAX_ORDER - 1;
        while (order > 0)
        {
            size_t block_size = BUDDY_BLOCK_SIZE(order);
            if (addr + block_size <= end && ((uint32_64)addr % block_size == 0))
            {
                break;
            }
            order--;
        }

        log_debug(MODULE, "  inserting addr=%p order=%d", addr, order);
        buddy_free(addr, order);
        addr += BUDDY_BLOCK_SIZE(order);
    }
}

void buddy_print_info()
{
    log_debug(MODULE, "=== Buddy Allocator Info ===");
    log_debug(MODULE, "Region: %p..%p (size: %x)", start, end, (uint32_64)(end - start));
    log_debug(MODULE, "");

    size_t total_free_bytes = 0;

    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++)
    {
        buddy_order *order = &global_buddy.orders[i];
        size_t block_size = BUDDY_BLOCK_SIZE(i);
        size_t free_bytes = order->count * block_size;
        total_free_bytes += free_bytes;

        log_debug(MODULE, "  order[%2u] block_size=%8x  count=%5u  free=%x bytes",
                  i, block_size, order->count, free_bytes);
    }

    log_debug(MODULE, "");
    log_debug(MODULE, "Total free: %x bytes (%u KiB / %u MiB)",
              total_free_bytes,
              (uint32_t)(total_free_bytes / 1024),
              (uint32_t)(total_free_bytes / (1024 * 1024)));
    log_debug(MODULE, "============================");
}

void buddy_print_info_verbose()
{
    buddy_print_info();

    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++)
    {
        buddy_order *order = &global_buddy.orders[i];
        if (order->count == 0)
            continue;

        log_debug(MODULE, "  order[%u] blocks:", i);
        for (size_t j = 0; j < order->count; j++)
        {
            log_debug(MODULE, "    [%4u] %p", j, order->blocks[j]);
        }
    }
}

void buddy_free(phys_addr physaddr, uint32_t order)
{
    log_debug(MODULE, "buddy_free addr=%p order=%d", physaddr, order);
    if (order >= BUDDY_MAX_ORDER)
        return;

    phys_addr addr = physaddr;

    while (order < BUDDY_MAX_ORDER - 1)
    {
        phys_addr buddy_addr = (phys_addr)((uint32_64)addr ^ BUDDY_BLOCK_SIZE(order));

        bool found = false;
        buddy_order *o = &global_buddy.orders[order];
        for (size_t i = 0; i < o->count; i++)
        {
            if (o->blocks[i] == buddy_addr)
            {
                // swap-remove
                o->blocks[i] = o->blocks[--o->count];
                addr = (addr < buddy_addr) ? addr : buddy_addr;
                order++;
                found = true;
                break;
            }
        }
        if (!found)
            break;
    }

    buddy_order *o = &global_buddy.orders[order];
    o->blocks[o->count++] = addr;
}

phys_addr buddy_alloc(uint32_t order)
{
    if (order >= BUDDY_MAX_ORDER)
        return 0;

    uint32_t found_order = order;
    while (found_order < BUDDY_MAX_ORDER && global_buddy.orders[found_order].count == 0)
        found_order++;

    if (found_order == BUDDY_MAX_ORDER)
    {
        log_err(MODULE, "OOM");
        return 0;
    }

    buddy_order *o = &global_buddy.orders[found_order];
    phys_addr addr = o->blocks[--o->count];

    while (found_order > order)
    {
        found_order--;
        phys_addr split = addr + BUDDY_BLOCK_SIZE(found_order);
        buddy_order *so = &global_buddy.orders[found_order];
        so->blocks[so->count++] = split;
    }

    log_debug(MODULE, "got addr=%p order=%d", addr, order);
    return addr;
}

phys_addr buddy_alloc_at(phys_addr addr)
{
    for (uint32_t order = BUDDY_MAX_ORDER - 1; order < BUDDY_MAX_ORDER; order--)
    {
        phys_addr block_size = (phys_addr)BUDDY_BLOCK_SIZE(order);
        phys_addr block_base = (phys_addr)ALIGN_DOWN((uint32_64)addr, (uint32_64)block_size);

        buddy_order *o = &global_buddy.orders[order];
        for (size_t i = 0; i < o->count; i++)
        {
            if (o->blocks[i] == block_base)
            {
                // swap-remove
                o->blocks[i] = o->blocks[--o->count];

                // split down, freeing halves that don't contain addr
                while (order > 0)
                {
                    order--;
                    block_size = (phys_addr)BUDDY_BLOCK_SIZE(order);
                    phys_addr upper = (phys_addr)((uint32_64)block_base + (uint32_64)block_size);
                    if (addr >= upper)
                    {
                        buddy_free(block_base, order);
                        block_base = upper;
                    }
                    else
                    {
                        buddy_free(upper, order);
                    }
                }
                return addr;
            }
        }
    }

    // not in any free list, already allocated
    return 0;
}
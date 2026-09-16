/*
 * File: buddy.c
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/pmm/frame_allocator.h"
#include "mm/pmm/pmm_register.h"
#include "mm/pmm/pmm_info.h"
#include "mm/pmm/pmm.h"

#include "debug/debug.h"

#include "asm/page_arch.h"

#include "math.h"
#include "align.h"
#include "memory.h"

#define MODULE                  "BUDDY"

#define BUDDY_MAX_ORDER         11 // 2^0 * 4KiB to 2^10 * 4KiB = 4KiB to 4MiB
#define MAX_SIZE                (1 << (BUDDY_MAX_ORDER - 1)) * PAGE_SIZE
#define LOW_SIZE                1 * PAGE_SIZE

#define BUDDY_BLOCK_SIZE(order) ((1 << order) * PAGE_SIZE)
#define MAX_ORDER0_BLOCKS       (0x40000000 / PAGE_SIZE)

typedef struct frame_allocator_stats
{
    size_t total_frames;
    size_t free_frames;
    size_t per_order_count[BUDDY_MAX_ORDER];

} frame_allocator_stats_t;

typedef struct buddy_free_node
{
    struct buddy_free_node *next;
    struct buddy_free_node *prev;
} buddy_free_node_t;

typedef struct buddy_order
{
    buddy_free_node_t *free_list;
    size_t count;
} buddy_order_t;

typedef struct buddy
{
    buddy_order_t orders[BUDDY_MAX_ORDER];
    paddr_t start;
    paddr_t end;
    size_t order_bit_offset[BUDDY_MAX_ORDER];
    paddr_t bitmap_phys;
    size_t bitmap_bytes;
    uint8_t *free_bitmap;
} buddy_t;

void buddy_print_info_verbose(frame_allocator_t *allocator);
void buddy_print_info(frame_allocator_t *allocator);
void buddy_stats(frame_allocator_t *allocator, frame_allocator_stats_t *out);
int buddy_free(frame_allocator_t *allocator, paddr_t physaddr, size_t nframes);
int buddy_alloc(frame_allocator_t *allocator, size_t nframes, paddr_t *out_phys);
int buddy_alloc_at(frame_allocator_t *allocator, paddr_t addr, size_t order, paddr_t *phys_out);

int buddy_free_block(frame_allocator_t *allocator, paddr_t physaddr, uint32_t order);

INLINE void list_push(buddy_order_t *o, buddy_free_node_t *node)
{
    node->prev = NULL;
    node->next = o->free_list;
    if (o->free_list)
    {
        o->free_list->prev = node;
    }
    o->free_list = node;
    o->count++;
}

INLINE void list_remove(buddy_order_t *o, buddy_free_node_t *node)
{
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        o->free_list = node->next;
    }
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    o->count--;
}

INLINE buddy_free_node_t *list_pop(buddy_order_t *o)
{
    buddy_free_node_t *node = o->free_list;
    if (node)
    {
        list_remove(o, node);
    }
    return node;
}

INLINE size_t buddy_block_index(buddy_t *global_buddy, paddr_t addr, size_t order)
{
    return ((addr - global_buddy->start) / PAGE_SIZE) >> order;
}

INLINE bool buddy_bit_test(buddy_t *global_buddy, size_t order, size_t index)
{
    size_t bit = global_buddy->order_bit_offset[order] + index;
    return (global_buddy->free_bitmap[bit / 8] >> (bit % 8)) & 1;
}

INLINE void buddy_bit_set(buddy_t *global_buddy, size_t order, size_t index)
{
    size_t bit = global_buddy->order_bit_offset[order] + index;
    global_buddy->free_bitmap[bit / 8] |= (1 << (bit % 8));
}

INLINE void buddy_bit_clear(buddy_t *global_buddy, size_t order, size_t index)
{
    size_t bit = global_buddy->order_bit_offset[order] + index;
    global_buddy->free_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

status_t buddy_setup(frame_allocator_t *out, pmm_info_t *info)
{
    ENTER_FUNC("%p, %p", out, info);
    paddr_t phys_buddy = pmm_alloc_frame();
    buddy_t *global_buddy = (buddy_t *)phys_to_virt_auto(phys_buddy);
    memset(global_buddy, 0, sizeof(buddy_t));

    size_t total_frames = PAGE_COUNT(info->phys_end - info->phys_start);
    size_t total_bits = 0;
    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++)
    {
        global_buddy->order_bit_offset[i] = total_bits;
        total_bits += (total_frames >> i) + 1;
    }
    size_t bitmap_bytes = PAGE_ALIGN_UP((total_bits + 7) / 8);
    size_t bitmap_frames = bitmap_bytes / PAGE_SIZE;

    paddr_t bitmap_phys = pmm_alloc_frame();
    for (size_t f = 1; f < bitmap_frames; f++)
    {
        pmm_alloc_frame();
    }

    global_buddy->bitmap_phys = bitmap_phys;
    global_buddy->bitmap_bytes = bitmap_bytes;

    out->priv = (void *)global_buddy;
    out->ops.stats = buddy_stats;
    out->ops.print_stat = buddy_print_info;
    out->ops.print_stat_verbose = buddy_print_info_verbose;
    out->ops.alloc = buddy_alloc;
    out->ops.free = buddy_free;
    out->ops.alloc_at = buddy_alloc_at;
    out->name = "buddy";

    return KERRNO_SUCCESSES;
}

status_t buddy_initialize(frame_allocator_t *out, pmm_info_t *info)
{
    ENTER_FUNC("%p, %p", out, info);
    buddy_t *global_buddy = out->priv;

    global_buddy->start = info->phys_start;
    global_buddy->end = info->phys_end;
    global_buddy->free_bitmap = (uint8_t *)phys_to_virt_auto(global_buddy->bitmap_phys);
    memset(global_buddy->free_bitmap, 0, global_buddy->bitmap_bytes);

    log_debug(MODULE, "region start=%p end=%p", global_buddy->start, global_buddy->end);
    log_debug(MODULE, "mapping the frames");
    log_debug(MODULE, "%p..%p size %x", global_buddy->start, global_buddy->end, global_buddy->end - global_buddy->start);

    paddr_t addr = global_buddy->start;
    while (addr < global_buddy->end)
    {
        uint32_t order = BUDDY_MAX_ORDER - 1;
        while (order > 0)
        {
            size_t block_size = BUDDY_BLOCK_SIZE(order);
            if (addr + block_size <= global_buddy->end && (addr % block_size == 0))
            {
                break;
            }
            order--;
        }

        log_debug(MODULE, "  inserting addr=%p order=%d", addr, order);
        buddy_free_block(out, addr, order);
        addr += BUDDY_BLOCK_SIZE(order);
    }
    return KERRNO_SUCCESSES;
}

void buddy_print_info_verbose(frame_allocator_t *allocator)
{
    buddy_t *global_buddy = allocator->priv;
    buddy_print_info(allocator);

    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++)
    {
        buddy_order_t *o = &global_buddy->orders[i];
        if (o->count == 0)
        {
            continue;
        }

        log_debug(MODULE, "  order[%u] blocks:", i);
        buddy_free_node_t *node = o->free_list;
        size_t j = 0;
        while (node)
        {
            paddr_t addr = virt_to_phys_auto((vaddr_t)node);
            log_debug(MODULE, "    [%4zu] %p/%p", j, addr, node);
            node = node->next;
            j++;
        }
    }
}

void buddy_print_info(frame_allocator_t *allocator)
{
    buddy_t *global_buddy = allocator->priv;
    frame_allocator_stats_t st;
    buddy_stats(allocator, &st);

    log_debug(MODULE, "=== Buddy Allocator Info ===");
    log_debug(MODULE, "Region: %p..%p (size: %x)", global_buddy->start, global_buddy->end, (uint32_t)(global_buddy->end - global_buddy->start));
    log_debug(MODULE, "");

    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++)
    {
        size_t block_size = BUDDY_BLOCK_SIZE(i);
        size_t free_bytes = st.per_order_count[i] * block_size;
        log_debug(MODULE, "  order[%2u] block_size=%8x  count=%5u  free=%x bytes", i, block_size, st.per_order_count[i], free_bytes);
    }

    log_debug(MODULE, "");
    size_t total_free_bytes = st.free_frames * PAGE_SIZE;
    size_t total_bytes = st.total_frames * PAGE_SIZE;
    log_debug(MODULE, "Total free: %x/%x bytes (%u KiB / %u MiB)", total_free_bytes, total_bytes, (uint32_t)(total_free_bytes / 1024), (uint32_t)(total_free_bytes / (1024 * 1024)));
    log_debug(MODULE, "============================");
}

void buddy_stats(frame_allocator_t *allocator, frame_allocator_stats_t *out)
{
    buddy_t *global_buddy = allocator->priv;
    memset(out, 0, sizeof(*out));
    out->total_frames = (global_buddy->end - global_buddy->start) / PAGE_SIZE;

    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++)
    {
        buddy_order_t *o = &global_buddy->orders[i];
        out->per_order_count[i] = o->count;
        out->free_frames += o->count * (1 << i);
    }
}

status_t buddy_free_block(frame_allocator_t *allocator, paddr_t physaddr, uint32_t order)
{
    buddy_t *global_buddy = allocator->priv;
    log_debug(MODULE, "buddy_free addr=%p order=%d", physaddr, order);
    if (order >= BUDDY_MAX_ORDER)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "order is not valid");
    }

    paddr_t addr = physaddr;

    while (order < BUDDY_MAX_ORDER - 1)
    {
        size_t idx = buddy_block_index(global_buddy, addr, order);
        size_t buddy_idx = idx ^ 1;
        paddr_t buddy_addr = global_buddy->start + (buddy_idx << order) * PAGE_SIZE;
        // log_debug(MODULE, "buddy_addr = %p", buddy_addr);

        if (buddy_addr + BUDDY_BLOCK_SIZE(order) > global_buddy->end)
        {
            break;
        }
        if (!buddy_bit_test(global_buddy, order, buddy_idx))
        {
            break;
        }

        buddy_free_node_t *buddy_node = (buddy_free_node_t *)phys_to_virt_auto(buddy_addr);
        // log_debug(MODULE, "buddy_node = %p", buddy_node);
        list_remove(&global_buddy->orders[order], buddy_node);
        buddy_bit_clear(global_buddy, order, buddy_idx);

        addr = (addr < buddy_addr) ? addr : buddy_addr;
        order++;
    }

    size_t idx = buddy_block_index(global_buddy, addr, order);
    // log_debug(MODULE, "idx = %u", idx);
    buddy_free_node_t *node = (buddy_free_node_t *)phys_to_virt_auto(addr);
    // log_debug(MODULE, "node = %p", node);
    list_push(&global_buddy->orders[order], node);
    buddy_bit_set(global_buddy, order, idx);
    return KERRNO_SUCCESSES;
}

status_t buddy_free(frame_allocator_t *allocator, paddr_t phys, size_t nframes)
{
    paddr_t addr = phys;
    size_t remaining = nframes;

    while (remaining > 0)
    {
        size_t max_align_order = trailing_zero_count(addr / PAGE_SIZE);
        size_t max_fit_order = floor_log2(remaining);
        size_t chunk_order = min(max_align_order, max_fit_order);

        buddy_free_block(allocator, addr, chunk_order);

        addr += (1 << chunk_order) * PAGE_SIZE;
        remaining -= (1 << chunk_order);
    }
    return KERRNO_SUCCESSES;
}

status_t buddy_alloc_block(frame_allocator_t *allocator, size_t order, paddr_t *phys_out)
{
    buddy_t *global_buddy = allocator->priv;
    if (order >= BUDDY_MAX_ORDER)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "order is not valid");
    }

    uint32_t found_order = order;
    while (found_order < BUDDY_MAX_ORDER && global_buddy->orders[found_order].count == 0)
    {
        found_order++;
    }

    if (found_order == BUDDY_MAX_ORDER)
    {
        KERRNO_RETURN(KERRNO_POSIX_ENOMEM, "Out of memory");
    }

    buddy_free_node_t *node = list_pop(&global_buddy->orders[found_order]);
    paddr_t addr = virt_to_phys_auto((vaddr_t)node);
    buddy_bit_clear(global_buddy, found_order, buddy_block_index(global_buddy, addr, found_order));

    while (found_order > order)
    {
        found_order--;
        paddr_t split = addr + BUDDY_BLOCK_SIZE(found_order);
        buddy_free_node_t *split_node = (buddy_free_node_t *)phys_to_virt_auto(split);
        list_push(&global_buddy->orders[found_order], split_node);
        buddy_bit_set(global_buddy, found_order, buddy_block_index(global_buddy, split, found_order));
    }

    log_debug(MODULE, "got addr=%p order=%d", addr, order);
    *phys_out = addr;
    return KERRNO_SUCCESSES;
}

status_t buddy_alloc(frame_allocator_t *allocator, size_t nframes, paddr_t *out_phys)
{
    size_t order = ceil_log2(nframes);

    paddr_t base;
    int state = buddy_alloc_block(allocator, order, &base);
    if (state != 0)
    {
        return state;
    }

    size_t allocated = 1 << order;
    size_t excess = allocated - nframes;

    if (excess > 0)
    {
        paddr_t addr = base + nframes * PAGE_SIZE;
        size_t remaining = excess;
        while (remaining > 0)
        {
            size_t max_align_order = trailing_zero_count(addr / PAGE_SIZE);
            size_t max_fit_order = floor_log2(remaining);
            size_t chunk_order = min(max_align_order, max_fit_order);
            buddy_free_block(allocator, addr, chunk_order);
            addr += (1 << chunk_order) * PAGE_SIZE;
            remaining -= (1 << chunk_order);
        }
    }

    *out_phys = base;
    return KERRNO_SUCCESSES;
}

static status_t buddy_alloc_at_single(frame_allocator_t *allocator, paddr_t addr)
{
    buddy_t *global_buddy = allocator->priv;
    for (uint32_t order = BUDDY_MAX_ORDER - 1;; order--)
    {
        paddr_t block_size = BUDDY_BLOCK_SIZE(order);
        paddr_t block_base = ALIGN_DOWN(addr, block_size);
        size_t idx = buddy_block_index(global_buddy, block_base, order);

        if (buddy_bit_test(global_buddy, order, idx))
        {
            buddy_free_node_t *node = (buddy_free_node_t *)phys_to_virt_auto(block_base);
            list_remove(&global_buddy->orders[order], node);
            buddy_bit_clear(global_buddy, order, idx);

            while (order > 0)
            {
                order--;
                block_size = BUDDY_BLOCK_SIZE(order);
                paddr_t upper = block_base + block_size;
                if (addr >= upper)
                {
                    buddy_free_block(allocator, block_base, order);
                    block_base = upper;
                }
                else
                {
                    buddy_free_block(allocator, upper, order);
                }
            }
            return KERRNO_SUCCESSES;
        }
        if (order == 0)
        {
            break;
        }
    }
    return 1; // already allocated / not in any free list
}

status_t buddy_alloc_at(frame_allocator_t *allocator, paddr_t addr, size_t nframes, paddr_t *phys_out)
{
    for (size_t i = 0; i < nframes; i++)
    {
        int rc = buddy_alloc_at_single(allocator, addr + i * PAGE_SIZE);
        if (rc != 0)
        {
            return rc;
        }
    }
    *phys_out = addr;
    return KERRNO_SUCCESSES;
}

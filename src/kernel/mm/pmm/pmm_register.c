/*
 * File: pmm_register.c
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/pmm/pmm_register.h"
#include "mm/pmm/pmm_info.h"
#include "kerrno.h"
#include "memory.h"
#include <config.h>
#include <types.h>
#include <defs.h>

#define MODULE         "pmm-register"

#define REGISTER(name) extern int name##_setup(frame_allocator_t *out, pmm_info_t *info);
#include "allocators.tbl"
#undef REGISTER

#define REGISTER(name) extern int name##_initialize(frame_allocator_t *alloc, pmm_info_t *info);
#include "allocators.tbl"
#undef REGISTER

#define REGISTER(name) ALLOCATOR_##name,

typedef enum
{

#include "allocators.tbl"
    ALLOCATOR_COUNT,
} allocator_index_t;

#undef REGISTER

typedef int (*alloc_setup_func)(frame_allocator_t *, pmm_info_t *);
typedef int (*alloc_initialize_func)(frame_allocator_t *, pmm_info_t *);

typedef struct alloc_funcs
{
    alloc_setup_func setup;
    alloc_initialize_func initialize;
} alloc_funcs_t;

#define REGISTER(name)                                                             \
    [ALLOCATOR_##name] = {.setup = name##_setup, .initialize = name##_initialize},
static alloc_funcs_t allocator_table[] = {
#include "allocators.tbl"
};
#undef REGISTER

status_t pmm_reg_get_allocator(pmm_info_t *info, frame_allocator_t table[CONFIG_MAX_FRAME_ALLOCATORS], int *out_index)
{
    *out_index = CONCAT(ALLOCATOR_, CONFIG_PMM_ALLOCATOR);
    for (size_t i = 0; i < ALLOCATOR_COUNT; i++)
    {
        if (i >= CONFIG_MAX_FRAME_ALLOCATORS)
        {
            KERRNO_RETURN(KERRNO_BAD_INDEX, "registry didn't have enough space");
        }
        frame_allocator_t func;
        log_debug(MODULE, "table[%u] @ %p, &func @ %p", i, table[i], &func);
        int state = allocator_table[i].setup(&func, info);
        if (state != 0)
        {
            KERRNO_RETURN(KERRNO_NO_INIT, "setup failed");
            return state;
        }
        log_debug(MODULE, "table[%u] @ %p, &func @ %p", i, table[i], &func);
        memcpy(&table[i], &func, sizeof(frame_allocator_t));
    }

    return KERRNO_SUCCESSES;
}

status_t pmm_reg_initialize(pmm_info_t *info, frame_allocator_t table[CONFIG_MAX_FRAME_ALLOCATORS])
{
    int index = CONCAT(ALLOCATOR_, CONFIG_PMM_ALLOCATOR);
    if (index >= CONFIG_MAX_FRAME_ALLOCATORS)
    {
        KERRNO_RETURN(KERRNO_BAD_INDEX, "registry didn't have enough space");
    }
    log_debug(MODULE, "table[%u] @ %p, &func @ %p", index, &table[index], info->allocator);
    int state = allocator_table[index].initialize(info->allocator, info);
    if (state != 0)
    {
        KERRNO_RETURN(KERRNO_NO_INIT, "initialize failed");
        return state;
    }
    return KERRNO_SUCCESSES;
}

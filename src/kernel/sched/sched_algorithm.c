/*
 * File: sched_algorithm.c
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sched/sched_algorithm.h"

#include "debug/debug.h"

#include "memory.h"

#include <config.h>

#define MODULE         "sched-algorithm"

#define REGISTER(name) extern int name##_setup();
#include "sched_algorithms.tbl"
#undef REGISTER

#define REGISTER(name) ALGORITHM_##name,

typedef enum algorithm_index
{

#include "sched_algorithms.tbl"
    ALGORITHM_COUNT,
} algorithm_index_t;

#undef REGISTER

typedef int (*algorithm_setup_func)(sched_class_t *);

typedef struct sched_algorithm_funcs
{
    algorithm_setup_func setup;
} sched_algorithm_funcs_t;

#define REGISTER(func)                 \
    [ALGORITHM_##func] = func##_setup,
static algorithm_setup_func algorithm_table[] = {
#include "sched_algorithms.tbl"
};
#undef REGISTER

status_t sched_algorithm_get_algorithm(sched_class_t algorithms[CONFIG_MAX_SCHED_ALGORITHMS], int *out_index)
{
    *out_index = CONCAT(ALGORITHM_, CONFIG_SCHED_ALGORITHM);
    for (size_t i = 0; i < ALGORITHM_COUNT; i++)
    {
        if (i >= CONFIG_MAX_SCHED_ALGORITHMS)
        {
            KERRNO_RETURN(KERRNO_BAD_INDEX, "registry didn't have enough space");
        }
        sched_class_t func;
        trace_debug(MODULE, "table[%u] @ %p, &func @ %p", i, algorithms[i], &func);
        int state = algorithm_table[i](&func);
        if (state != KERRNO_SUCCESSES)
        {
            KERRNO_RETURN(KERRNO_NO_INIT, "setup failed");
            return state;
        }
        trace_debug(MODULE, "table[%u] @ %p, &func @ %p", i, algorithms[i], &func);
        memcpy(&algorithms[i], &func, sizeof(sched_class_t));
    }

    return KERRNO_SUCCESSES;
}


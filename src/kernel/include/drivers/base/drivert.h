/*
 * File: drivert.h
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "core_types.h"

#include "module.h"

#include <types.h>

#define MODULE_DRIVER(__driver, __register, __unregister, ...) \
    static status_t __init __driver##_init()                   \
    {                                                          \
        return __register(&(__driver));                        \
    }                                                          \
    MODULE_INIT(__driver##_init)

// static void __exit __driver##_exit()
// {
// __unregister(&(__driver) , ##__VA_ARGS__);
// }
// module_exit(__driver##_exit);

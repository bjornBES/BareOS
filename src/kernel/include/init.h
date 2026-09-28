/*
 * File: init.h
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>
#include <boot/params.h>

#include <types.h>

typedef status_t (*initcall_t)();
typedef void (*exitcall_t)();

#define DEFINE_INITCALL_WITH_ID(fn, id, __sec)                                                                   \
    static USED SECTION(STRINGIFY(__sec) ".init") initcall_t CONCAT(initcall_##fn, CONCAT(_, __COUNTER__)) = fn;

#define DEFINE_INITCALL(fn, id) DEFINE_INITCALL_WITH_ID(fn, (id), CONCAT(.initcall, id))

#define INITCALL_LVL_EARLY      early
#define INITCALL_LVL_ARCH       1
#define INITCALL_LVL_CORE       2
#define INITCALL_LVL_POSTCORE   3
#define INITCALL_LVL_SUBSYS     4
#define INITCALL_LVL_FS         5
#define INITCALL_LVL_ROOTFS     rootfs
#define INITCALL_LVL_DEVICE     6
#define INITCALL_LVL_PREUSER    7

#define _CALL_INITCALL_FUNCTIONS(_name_)                                                   \
    {                                                                                      \
        extern char _name_;                                                                \
        initcall_t *funcs = (initcall_t *)&_name_;                                         \
        while (*funcs != NULL)                                                             \
        {                                                                                  \
            status_t ret = (*funcs)();                                                     \
            if (ret != KERRNO_SUCCESSES)                                                   \
            {                                                                              \
                KERNEL_PANIC("initcall", "init function at %p returned non zero", *funcs); \
            }                                                                              \
            funcs++;                                                                       \
        }                                                                                  \
    }

#define CALL_INITCALL_FUNCTIONS(lvl) _CALL_INITCALL_FUNCTIONS(CONCAT(initcall, CONCAT(lvl, _start)))

#define EARLY_INITCALL(fn)           DEFINE_INITCALL(fn, INITCALL_LVL_EARLY)
#define ARCH_INITCALL(fn)            DEFINE_INITCALL(fn, INITCALL_LVL_ARCH)
#define CORE_INITCALL(fn)            DEFINE_INITCALL(fn, INITCALL_LVL_CORE)
#define POSTCORE_INITCALL(fn)        DEFINE_INITCALL(fn, INITCALL_LVL_POSTCORE)
#define SUBSYS_INITCALL(fn)          DEFINE_INITCALL(fn, INITCALL_LVL_SUBSYS)
#define FS_INITCALL(fn)              DEFINE_INITCALL(fn, INITCALL_LVL_FS)
#define ROOTFS_INITCALL(fn)          DEFINE_INITCALL(fn, INITCALL_LVL_ROOTFS)
#define DEVICE_INITCALL(fn)          DEFINE_INITCALL(fn, INITCALL_LVL_DEVICE)
#define PREUSER_INITCALL(fn)         DEFINE_INITCALL(fn, INITCALL_LVL_PREUSER)

#define INITCALL(fn)                 DEVICE_INITCALL(fn)

void kernel_early_main(boot_params_t *boot_params);
void kernel_main();

/* sections */

// These is for kernel start up until an arch calls kernel_main in main.c
#define __init SECTION(".init.text") COLD

/*
 * File: vfs.c
 * File Created: 26 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/vfs.h"

#include "kerrno.h"

#include "init.h"

status_t vfs_initialize()
{
    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_FS);

    return KERRNO_SUCCESSES;
}
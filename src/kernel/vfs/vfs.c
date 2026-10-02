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
#include "vfs/vfs_internals.h"

#include "kerrno.h"

#include "init.h"

#define MODULE "vfs"

status_t vfs_initialize()
{
    ENTER_FUNC("", 0);
    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_FS);

    return KERRNO_SUCCESSES;
}

status_t vfs_mount(const char *label, device_t *dev, int flags)
{
    return KERRNO_UNSUCCESS;    
}
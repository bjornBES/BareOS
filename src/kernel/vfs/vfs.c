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
#include "vfs/fs_registry.h"
#include "vfs/volume.h"
#include "vfs/path.h"
#include "vfs/mount.h"

#include "stdio.h"

#include "kerrno.h"

#include "init.h"

#define MODULE "vfs"

extern void hexdump(void *ptr, size_t len, size_t size);

status_t vfs_initialize()
{
    ENTER_FUNC("", 0);
    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_FS);

    return KERRNO_SUCCESSES;
}

INTERNAL status_t vfs_do_mount(const char *path, device_t *dev, volume_t *volume, int flags)
{
    ENTER_FUNC("%s, %p, %p, 0x%x", path, dev, volume, flags);

    mountpoint_t *mount = NULL;
    status_t ret = mount_create(volume, path, flags, &mount);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    return KERRNO_SUCCESSES;
}

status_t vfs_mount_label(const char *label, device_t *dev, int flags)
{
    return KERRNO_UNSUCCESS;
}

status_t vfs_mount(device_t *dev, int flags)
{
    ENTER_FUNC("%p, 0x%x", dev, flags);
    filesystem_t *fs;
    status_t ret = vfs_fs_probe(dev, &fs);
    // KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    if (!fs->ident_fs)
    {
        KERRNO_RETURN(KERRNO_UNSUCCESS, "%s dose not have ident_fs set", fs->name);
    }

    blkdev_t *block_dev = dev->class_data;

    if (fs->ident_fs(dev, &block_dev->fs_ident) != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_UNSUCCESS, "%s did not identify itself", fs->name);
    }

    hexdump(&block_dev->fs_ident, sizeof(fs_ident_t), 16);

    volume_t *vol = NULL;
    ret = volume_do_register(block_dev->fs_ident.label, dev, fs, &vol);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    trace_debug(MODULE, "vol = %p", vol);
    char path[255];
    size_t path_count = 0;
#if CONFIG_VOLUME_POSTFIX != 0 && CONFIG_VOLUME_PREFIX != 0
    sprintf(path, "%c%c%s%c%c", )
#elif CONFIG_VOLUME_POSTFIX != 0
    sprintf(path, "%c%c%s%c%c", )
#elif CONFIG_VOLUME_PREFIX != 0
    path_count = sprintf(path, "%c%c%s%c", CONFIG_PATH_DIR_SEPARATOR, CONFIG_VOLUME_PREFIX, block_dev->fs_ident.label, CONFIG_PATH_DIR_SEPARATOR);
#else
#error "both CONFIG_VOLUME_PREFIX and CONFIG_VOLUME_POSTFIX are 0"
#endif

        path[path_count] = '\0';

    ret = vfs_do_mount(path, dev, vol, flags);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    return KERRNO_SUCCESSES;
}

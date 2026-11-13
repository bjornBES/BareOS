/*
 * File: devfs.c
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "devfs.h"
#include "devfst.h"
#include "vfs/fs_registry.h"
#include "vfs/volume.h"
#include "vfs/vfs.h"

#include "dev/device.h"

#include "drivers/base/core.h"
#include "drivers/base/class.h"

#include "memory.h"
#include "string.h"

#include "kerrno.h"

#include "init.h"

#include "stdio.h"

#include "lists/hash.h"

#define MODULE "devfs"

hash_map_t *devfs_map;
volume_t *devfs_vol;

status_t devfs_probe(device_t *dev)
{
    if (dev->class->type == virtual_class.type)
    {
        return KERRNO_SUCCESSES;
    }
    return KERRNO_UNSUCCESS;
}

status_t devfs_mount(device_t *dev, mountpoint_t *mnt)
{
    devfs_vol = mnt->volume;
    devfs_priv_data_t *priv = kzmalloc(sizeof(devfs_priv_data_t));
    hash_map_create(&devfs_map, 64);
    devfs_vol->sb = priv;
    return KERRNO_SUCCESSES;
}

status_t devfs_lookup(inode_t *parent, const char *name, inode_t *out, device_t *dev, mountpoint_t *mnt)
{
    ENTER_FUNC("%p, %s, %p, %p, %p", parent, name, out, dev, mnt);

    devfs_priv_data_t *sb = (devfs_priv_data_t *)mnt->volume->sb;

    dev_inode_t *dev_ino = (dev_inode_t *)out;
    if (parent == NULL && strcmp(name, "/") == 0)
    {
        // root inode request
        out->ino = 0;
        out->type = DT_DIR;
        out->size = 0;
        dev_ino->map = devfs_map;
        return KERRNO_SUCCESSES;
    }

    dev_inode_t *dev_parent = (dev_inode_t *)parent;
    hash_node_t *node;
    hash_search(dev_parent->map, (char *)name, &node);
    inode_t *inode = node->value;
    dev_inode_t *dev_inode = (dev_inode_t *)inode;
    out->type = inode->type;
    trace_debug(MODULE, "dev_inode->dev_id = ");
    device_id_print(&dev_inode->dev_id);
    memcpy(dev_ino, dev_inode, sizeof(dev_inode_t));
    return KERRNO_SUCCESSES;
}

status_t devfs_open(vfs_node_t *node, device_t *dev, mountpoint_t *mnt)
{
    dev_inode_t *dev_ino = (dev_inode_t *)node->inode;
    device_t *target_dev;
    status_t ret = device_get_device(&dev_ino->dev_id, &target_dev);
    KERRNO_CHECK_RETURN(ret);
    device_print_debug(target_dev);
    return target_dev->ops.open(target_dev, dev_ino->base.flags, node);
}

status_t devfs_write(vfs_node_t *node, const void *buf, off_t offset, size_t len, ssize_t *len_out, device_t *dev, mountpoint_t *mnt)
{
    ENTER_FUNC("%p, %p, 0x%x, 0x%x, %p, %p, %p", node, buf, offset, len, len_out, dev, mnt);
    dev_inode_t *dev_ino = (dev_inode_t *)node->inode;
    device_t *target_dev;
    status_t ret = device_get_device(&dev_ino->dev_id, &target_dev);
    KERRNO_CHECK_RETURN(ret);
    if (ret)
    {
        device_id_print(&dev_ino->dev_id);
        KERRNO_RETURN(KERRNO_DEVICE_NOT_FOUND, "device was not found");
    }
    if (!target_dev->ops.write)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "device \"%s\" doesn't have write", target_dev->name);
    }
    return target_dev->ops.write(target_dev, buf, len, offset, len_out);
}

status_t devfs_alloc_inode(volume_t *vol, inode_t **out)
{
    dev_inode_t *ino = kzmalloc(sizeof(dev_inode_t));
    ino->base.volume = vol;
    ino->base.fs = vol->fs;
    *out = &ino->base;
    return hash_map_create(&ino->map, 64);
}

status_t devfs_free_inode(inode_t *ino)
{
    dev_inode_t *dev_ino = (dev_inode_t *)ino;
    // kfree(dev_ino->cluster_chain);
    return kfree(dev_ino);
}

static filesystem_t dev_fs = {
    .name = "devfs",
    .probe = devfs_probe,

    // .ident_fs = devfs_ident_fs,

    .mount = devfs_mount,

    .lookup = devfs_lookup,

    .open = devfs_open,
    // .close = devfs_close,
    // .read = devfs_read_file,
    .write = devfs_write,

    .alloc_inode = devfs_alloc_inode,
    .free_inode = devfs_free_inode,
};

VFS_FS_DRIVER(dev_fs);

status_t devfs_add(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    device_print_debug(dev);
    inode_t *inode;
    devfs_alloc_inode(devfs_vol, &inode);
    dev_inode_t *dev_inode = to_dev_inode(inode);
    dev_inode->dev_id.id = dev->devt.id;
    dev_inode->is_char = dev_is_char(dev);
    dev_inode->is_blk = !dev_is_char(dev);
    if (dev_inode->is_alias)
    {
        inode->type = DT_DIR;
    }
    else if (dev_inode->is_char)
    {
        inode->type = DT_CHR;
    }
    else
    {
        inode->type = DT_BLK;
    }
    return hash_insert(devfs_map, dev->name, inode);
}

status_t devfs_device_added(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    return KERRNO_SUCCESSES;
}

status_t devfs_device_removed(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    return KERRNO_SUCCESSES;
}

const dev_class_t virtual_class = {
    .name = "virtual",
    .type = DEVICE_VIRTUAL,
    .device_added = devfs_device_added,
    .device_removed = devfs_device_removed,
};

status_t devfs_initcall_init()
{
    device_t *dev = core_device_create(&virtual_class, NULL, NULL, "devfs");
    char *path = kzmalloc(CONFIG_MAX_VOLUME_LENGTH);
    size_t path_count = 0;
#if CONFIG_VOLUME_POSTFIX != 0 && CONFIG_VOLUME_PREFIX != 0
    path_count = sprintf(path, "%c%cDEVFS%c%c", CONFIG_PATH_DIR_SEPARATOR, CONFIG_VOLUME_PREFIX, CONFIG_VOLUME_POSTFIX, CONFIG_PATH_DIR_SEPARATOR);
#elif CONFIG_VOLUME_POSTFIX != 0
    sprintf(path, "%c%cDEVFS%c%c", )
#elif CONFIG_VOLUME_PREFIX != 0
    path_count = sprintf(path, "%c%cDEVFS%c", CONFIG_PATH_DIR_SEPARATOR, CONFIG_VOLUME_PREFIX, CONFIG_PATH_DIR_SEPARATOR);
#else
#error "both CONFIG_VOLUME_PREFIX and CONFIG_VOLUME_POSTFIX are 0"
#endif
    path[path_count] = '\0';
    vfs_mount_label(path, dev, 0);
    return KERRNO_SUCCESSES;
}
ROOTFS_INITCALL(devfs_initcall_init)


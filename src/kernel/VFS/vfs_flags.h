/*
 * File: vfs_flags.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "volume_flags.h"
#include "dentry_flags.h"
#include "mount_flags.h"
#include "inode_flags.h"


// read only
#define VFS_O_RDONLY 0x0000

// write only
#define VFS_O_WRONLY 0x0001

// read + write
#define VFS_O_RDWR 0x0002

// create if not exists
#define VFS_O_CREAT 0x0004

// truncate to zero on open
#define VFS_O_TRUNC 0x0008

// writes always go to end
#define VFS_O_APPEND 0x0010

// fail if file already exists (with O_CREAT)
#define VFS_O_EXCL 0x0020

// don't follow symlinks
#define VFS_O_NOFOLLOW 0x0040

// fail if not a directory
#define VFS_O_DIRECTORY 0x0080

// owner read
#define VFS_S_IRUSR 0x0100

// owner write
#define VFS_S_IWUSR 0x0200

// owner execute
#define VFS_S_IXUSR 0x0400

// group read
#define VFS_S_IRGRP 0x0010

// group write
#define VFS_S_IWGRP 0x0020

// group execute
#define VFS_S_IXGRP 0x0040

// other read
#define VFS_S_IROTH 0x0001

// other write
#define VFS_S_IWOTH 0x0002

// other execute
#define VFS_S_IXOTH 0x0004

// common combos
#define VFS_S_IRWXU VFS_S_IRUSR | VFS_S_IWUSR | VFS_S_IXUSR,
#define VFS_S_IRWXG VFS_S_IRGRP | VFS_S_IWGRP | VFS_S_IXGRP,

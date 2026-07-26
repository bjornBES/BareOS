/*
 * File: stat.h
 * File Created: 25 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct stat
{
    dev_t st_dev;         /* ID of device containing file */
    ino_t st_ino;         /* Inode number */
    nlink_t st_nlink;     /* Number of hard links */

    mode_t st_mode;       /* File type and mode */
    uid_t st_uid;         /* User ID of owner */
    gid_t st_gid;         /* Group ID of owner */
    uint32_t __pad0;
    dev_t st_rdev;        /* Device ID (if special file) */
    off_t st_size;        /* Total size, in bytes */
    blksize_t st_blksize; /* Block size for filesystem I/O */
    blkcnt_t st_blocks;   /* Number of 512 B blocks allocated */

    timespec_t st_atim;   /* Time of last access */
    timespec_t st_mtim;   /* Time of last modification */
    timespec_t st_ctim;   /* Time of last status change */
    long __unused[3];

} stat_t;

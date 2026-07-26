/*
 * File: vfs_sys.h
 * File Created: 25 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define O_TTY_INIT  0
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0040
#define O_EXCL      0x0080
#define O_NOCTTY    0x0100
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
#define O_NONBLOCK  0x0800
#define O_NDELAY    O_NONBLOCK
#define O_DSYNC     0x1000
#define O_ASYNC     0x2000
#define O_DIRECT    0x4000
#define O_LARGEFILE 0x8000
#define O_DIRECTORY 0x10000
#define O_NOFOLLOW  0x20000
#define O_NOATIME   0x40000
#define O_CLOEXEC   0x80000
#define O_SYNC      0x101000
#define O_RSYNC     0x101000
#define O_PATH      0x200000
#define O_SEARCH    O_PATH
#define O_EXEC      O_PATH
#define O_TMPFILE   0x410000
#define O_ACCMODE   (03 | O_SEARCH)

#define F_DUPFD     0
#define F_GETFD     1
#define F_SETFD     2
#define F_GETFL     3
#define F_SETFL     4
#define F_SETOWN    8
#define F_GETOWN    9
#define F_SETSIG    10
#define F_GETSIG    11
#if __LONG_MAX == 0x7FFFFFFFl
#define F_GETLK  12
#define F_SETLK  13
#define F_SETLKW 14
#else
#define F_GETLK  5
#define F_SETLK  6
#define F_SETLKW 7
#endif
#define F_SETOWN_EX            15
#define F_GETOWN_EX            16
#define F_GETOWNER_UIDS        17
#define F_OFD_GETLK            0x24
#define F_OFD_SETLK            0x25
#define F_OFD_SETLKW           0x26
#define F_DUPFD_CLOEXEC        1030
#define F_RDLCK                0x00
#define F_WRLCK                0x01
#define F_UNLCK                0x02

#define FD_CLOEXEC             0x01

#define AT_NO_AUTOMOUNT        0x800
#define AT_EMPTY_PATH          0x1000
#define AT_STATX_SYNC_TYPE     0x6000
#define AT_STATX_SYNC_AS_STAT  0x0000
#define AT_STATX_FORCE_SYNC    0x2000
#define AT_STATX_DONT_SYNC     0x4000
#define AT_RECURSIVE           0x8000
#define AT_FDCWD               (-100)
#define AT_SYMLINK_NOFOLLOW    0x100
#define AT_REMOVEDIR           0x200
#define AT_SYMLINK_FOLLOW      0x400
#define AT_EACCESS             0x200

#define POSIX_FADV_NORMAL      0
#define POSIX_FADV_RANDOM      1
#define POSIX_FADV_SEQUENTIAL  2
#define POSIX_FADV_WILLNEED    3
#define POSIX_FADV_DONTNEED    4
#define POSIX_FADV_NOREUSE     5

#define SEEK_SET               0
#define SEEK_CUR               1
#define SEEK_END               2

#define S_IXOTH                0x001
#define S_IWOTH                0x002
#define S_IROTH                0x004
#define S_IRWXO                S_IROTH | S_IWOTH | S_IXOTH
#define S_IXGRP                0x008
#define S_IWGRP                0x010
#define S_IRGRP                0x020
#define S_IRWXG                S_IRGRP | S_IWGRP | S_IXGRP
#define S_IXUSR                0x040
#define S_IWUSR                0x080
#define S_IRUSR                0x100
#define S_IRWXU                S_IRUSR | S_IWUSR | S_IXUSR
#define S_ISVTX                0x200
#define S_ISGID                0x400
#define S_ISUID                0x800

#define FAPPEND                O_APPEND
#define FFSYNC                 O_SYNC
#define FASYNC                 O_ASYNC
#define FNONBLOCK              O_NONBLOCK
#define FNDELAY                O_NDELAY

#define F_OK                   0
#define R_OK                   4
#define W_OK                   2
#define X_OK                   1
#define F_ULOCK                0
#define F_LOCK                 1
#define F_TLOCK                2
#define F_TEST                 3

#define F_SETLEASE             0x400
#define F_GETLEASE             0x401
#define F_NOTIFY               0x402
#define F_CANCELLK             0x405
#define F_SETPIPE_SZ           0x407
#define F_GETPIPE_SZ           0x408
#define F_ADD_SEALS            0x409
#define F_GET_SEALS            0x40A

#define F_SEAL_SEAL            0x0001
#define F_SEAL_SHRINK          0x0002
#define F_SEAL_GROW            0x0004
#define F_SEAL_WRITE           0x0008
#define F_SEAL_FUTURE_WRITE    0x0010

#define F_GET_RW_HINT          0x40B
#define F_SET_RW_HINT          0x40C
#define F_GET_FILE_RW_HINT     0x40D
#define F_SET_FILE_RW_HINT     0x40E

#define RWF_WRITE_LIFE_NOT_SET 0x00
#define RWH_WRITE_LIFE_NONE    0x01
#define RWH_WRITE_LIFE_SHORT   0x02
#define RWH_WRITE_LIFE_MEDIUM  0x03
#define RWH_WRITE_LIFE_LONG    0x04
#define RWH_WRITE_LIFE_EXTREME 0x05

#define DN_ACCESS              0x00000001
#define DN_MODIFY              0x00000002
#define DN_CREATE              0x00000004
#define DN_DELETE              0x00000008
#define DN_RENAME              0x00000010
#define DN_ATTRIB              0x00000020
#define DN_MULTISHOT           0x80000000

#define S_IFREG                0x8000

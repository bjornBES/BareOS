/*
 * File: mman.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define PROT_READ 0x1  /* Page can be read.  */
#define PROT_WRITE 0x2 /* Page can be written.  */
#define PROT_EXEC 0x4  /* Page can be executed.  */
#define PROT_NONE 0x0  /* Page can not be accessed.  */

#define MAP_SHARED 0x01    /* Share changes.  */
#define MAP_PRIVATE 0x02   /* Changes are private.  */
#define MAP_ANONYMOUS 0x20 /* Don't use a file.  */

void *mmap(void *addr, size_t length, int prot, int flags, fd_t fd, off_t offset);

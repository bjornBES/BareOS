#pragma once
#include <kernel/vfsBase.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

int VFS_Write(fd_t file, uint8_t* data, size_t size);
int VFS_Read(fd_t file, uint8_t* data, size_t size);
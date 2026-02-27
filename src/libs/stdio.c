/*
 * File: stdio.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stdio.h"

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>
#include <core/printfDriver/printf.h>
#include <core/arch/i686/VGATextDevice.h>
#include <core/arch/i686/e9.h>
#include <kernel/vfsBase.h>

#define MODULE "stdio"

extern int VFS_Write(fd_t file, uint8_t *data, size_t size);
extern int VFS_Read(fd_t file, uint8_t *data, size_t size);

void clear()
{
    Clear();
}

void putc(char c)
{
    fputc(c, stdout);
}

void puts(const char *str)
{
    fputs(str, stdout);
}

void fputc(char c, fd_t file)
{
    VFS_Write(file, (uint8_t*)&c, 1);
}

void fputs(const char *str, fd_t file)
{
    int len = strlen(str);
    VFS_Write(file, (uint8_t*)str, len);
}

int vfprintf(fd_t file, const char *fmt, va_list args)
{
    return vprintf(file, fmt, args);
}

int fprintf(fd_t file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(file, fmt, args);
    va_end(args);
    return ret;
}

const char g_HexChars[] = "0123456789abcdef";

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(VFS_FD_STDOUT, fmt, args);
    va_end(args);
    return ret;
}

int sprintf(char *s, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const int ret = vsprintf(s, format, args);
    va_end(args);
    return ret;
}

int snprintf(char *s, size_t n, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const int ret = vsnprintf(s, n, format, args);
    va_end(args);
    return ret;
}

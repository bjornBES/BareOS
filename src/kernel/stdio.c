/*
 * File: stdio.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stdio.h"
#include "VFS/vfs.h"
#include "kernel/string.h"
#include "kernel/debug.h"

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#include <printf_driver/printf.h>

#define MODULE "stdio"

int fputc(char c, fd_t file)
{
    return vfs_write(file, (uint8_t *)&c, 1);
}

int fputs(const char *str, fd_t file)
{
    int len = strlen(str);
    return vfs_write(file, (uint8_t *)str, len);
}

int putc(char c, fd_t file)
{
    return fputc(c, file);
}

int putchar(int c)
{
    return fputc(c, stdout);
}

int puts(const char *str)
{
    return fputs(str, stdout);
}


fd_t fopen(char *filename, const char *mode)
{
    return vfs_open(filename, 0, 0);
}

// fd_t fdopen(int fildes, const char *mode);
// fd_t popen(const char *command, const char *mode);
// fd_t freopen(const char *filename, const char *mode, fd_t file);

int fclose(fd_t file)
{
    return vfs_close(file);
}

// int pclose(fd_t file);

int fileno(fd_t file)
{
    return 0;
}

int feof(fd_t file)
{
    return 0;
}

int ferror(fd_t file)
{
    return 0;
}

int fflush(fd_t file)
{
    return 0;
}


int fseek(fd_t file, int64_t offset, int whence)
{
    return vfs_seek(file, (off_t)offset, whence);
}

int fseeko(fd_t file, off_t offset, int whence)
{
    return 0;
}


int64_t ftell(fd_t stream)
{
    return 0;
}

off_t ftello(fd_t stream)
{
    return 0;
}


size_t fread(void *buffer, size_t size, size_t count, fd_t stream)
{
    size_t r_size = size * count;
    return vfs_read(stream, (uint8_t*)buffer, r_size);
}

size_t fwrite(const void *buffer, size_t size, size_t count, fd_t stream)
{
    size_t r_size = size * count;
    return vfs_write(stream, (uint8_t*)buffer, r_size);
}

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf_int(stdout, fmt, args);
    va_end(args);
    return ret;
}

int fprintf(fd_t file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf_int(file, fmt, args);
    va_end(args);
    return ret;
}

int sprintf(char *s, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int ret = vsprintf_int(s, fmt, args);
    va_end(args);
    return ret;
}

int snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int ret = vsnprintf_int(s, n, fmt, args);
    va_end(args);
    return ret;
}

int vprintf(fd_t file, const char *fmt, va_list args)
{
    return vprintf_int(file, fmt, args);
}

int vfprintf(fd_t file, const char *fmt, va_list args)
{
    return vprintf_int(file, fmt, args);
}

int vsprintf(char *s, const char *fmt, va_list args)
{
    return vsprintf_int(s, fmt, args);
}

int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
    return vsnprintf_int(s, n, fmt, args);
}


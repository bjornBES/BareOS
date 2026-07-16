/*
 * File: stdio.c
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stdio.h"
#include "unistd.h"
#include <printf_driver/printf.h>
#include "syscall.h"

void fputc(char c, fd_t fd)
{
    write(fd, &c, 1);
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
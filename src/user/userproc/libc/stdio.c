/*
 * File: stdio.c
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Apr 2026
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

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(stdout, fmt, args);
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
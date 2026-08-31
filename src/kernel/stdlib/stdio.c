/*
 * File: stdio.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stdio.h"

#include "module.h"

#define MODULE ENTER_MODULE("", "stdio")

int fputc(char c, fd_t file)
{
    // return vfs_write(file, (uint8_t *)&c, 1);
    return 0;
}

int fputs(const char *str, fd_t file)
{
    // int len = strlen(str);
    // return vfs_write(file, (uint8_t *)str, len);
    return 0;
}

int putc(char c, fd_t file)
{
    // return fputc(c, file);
    return 0;
}

int putchar(int c)
{
    // return fputc(c, stdout);
    return 0;
}

int puts(const char *str)
{
    // return fputs(str, stdout);
    return 0;
}



int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // spinlock_acquire(&printf_lock);
    int ret = vprintf_int(STDOUT_FD, fmt, args);
    // spinlock_release(&printf_lock);
    va_end(args);
    return ret;
}

int fprintf_internal(fd_t file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // spinlock_acquire(&printf_lock);
    int ret = vprintf_int(file, fmt, args);
    // spinlock_release(&printf_lock);
    va_end(args);
    return ret;
}

int fprintf(fd_t file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // spinlock_acquire(&printf_lock);
    int ret = vprintf_int(file, fmt, args);
    // spinlock_release(&printf_lock);
    va_end(args);
    return ret;
}

int sprintf(char *s, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // irq_arch_disable();
    const int ret = vsprintf_int(s, fmt, args);
    // irq_arch_enable();
    va_end(args);
    return ret;
}

int snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // irq_arch_disable();
    const int ret = vsnprintf_int(s, n, fmt, args);
    // irq_arch_enable();
    va_end(args);
    return ret;
}

int vprintf(fd_t file, const char *fmt, va_list args)
{
    // spinlock_acquire(&printf_lock);
    int state = vprintf_int(file, fmt, args);
    // spinlock_release(&printf_lock);
    return state;
}

int vfprintf(fd_t file, const char *fmt, va_list args)
{
    // spinlock_acquire(&printf_lock);
    int state = vprintf_int(file, fmt, args);
    // spinlock_release(&printf_lock);
    return state;
}

int vsprintf(char *s, const char *fmt, va_list args)
{
    // irq_arch_disable();
    int state = vsprintf_int(s, fmt, args);
    // irq_arch_enable();
    return state;
}

int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
    // irq_arch_disable();
    int state = vsnprintf_int(s, n, fmt, args);
    // irq_arch_enable();
    return state;
}
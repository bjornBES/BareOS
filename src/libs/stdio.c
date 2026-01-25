#include "stdio.h"

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>
#include <core/printfDriver/printf.h>
#include <core/arch/i686/VGATextDevice.h>
#include <kernel/vfs.h>
#include <core/arch/i686/e9.h>

#define MODULE "stdio"

void putc(char c)
{
    fputc(c, stdout);
}

void puts(const char* str)
{
    fputs(str, stdout);
}

void fputc(char c, fd_t file)
{
    if (file == stdout)
    {
        putChar(c);
    }
    if (file == stddebug)
    {
        e9_putc(c);
    }
}

void fputs(const char* str, fd_t file)
{
    while(*str)
    {
        fputc(*str, file);
        str++;
    }
}

int vfprintf(fd_t file, const char* fmt, va_list args)
{
    return vprintf(file, fmt, args);
}

int fprintf(fd_t file, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(file, fmt, args);
    va_end(args);
    return ret;
}

const char g_HexChars[] = "0123456789abcdef";

int printf(const char* fmt, ...)
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

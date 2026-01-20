#include <internal_stdio.h>
#include <IO.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include <core/printfDriver/printf.h>
#include <core/arch/i686/VGATextDevice.h>

const char g_HexChars[] = "0123456789abcdef";


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
/*


void fprint_buffer(fd_t file, const char* msg, const void* buffer, uint32_t count)
{
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    
    fputs(msg, file);
    for (uint16_t i = 0; i < count; i++)
    {
        fputc(g_HexChars[u8Buffer[i] >> 4], file);
        fputc(g_HexChars[u8Buffer[i] & 0xF], file);
    }
    fputs("\n", file);
}

char putc(char c)
{
    fputc(c, VFS_FD_STDOUT);
    return c;
}

int puts(const char* str)
{
    return fputs(str, VFS_FD_STDOUT);
}

*/
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

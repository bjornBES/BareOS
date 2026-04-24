/*
 * File: stdio.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stdio.h"
#include "x86.h"
#include "IO.h"

#include <stdarg.h>
#include <stdbool.h>

bool disableOutput = false;
uint16_t ScreenWidth = 80;
uint16_t ScreenHeight = 25;
uint8_t DefaultColor = 0x7;
uint8_t* g_ScreenBuffer = (uint8_t*)0xB8000;
int m_ScreenX, m_ScreenY;

char vga_get_cell(int x, int y, uint8_t *color)
{
    int index = 2 * (y * ScreenWidth + x);
    char c = g_ScreenBuffer[index];
    *color = g_ScreenBuffer[index + 1];
    return c;
}
void vga_set_cell(int x, int y, char c, uint8_t color)
{
    int index = 2 * (y * ScreenWidth + x);
    g_ScreenBuffer[index] = c;
    g_ScreenBuffer[index + 1] = color;
}

void vga_set_cursor(int x, int y)
{
    int pos = y * ScreenWidth + x;
    m_ScreenX = x;
    m_ScreenY = y;

    Outb(0x3D4, 0x0F);
    Outb(0x3D5, (uint8_t)(pos & 0xFF));
    Outb(0x3D4, 0x0E);
    Outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_get_cursor(int* x, int* y)
{
    *x = m_ScreenX;
    *y = m_ScreenY;
}
void vga_clear()
{
    for (int y = 0; y < ScreenHeight; y++)
        for (int x = 0; x < ScreenWidth; x++)
        {
            vga_set_cell(x,y,' ', DefaultColor);
        }
    m_ScreenX = 0;
    m_ScreenY = 0;
    vga_set_cursor(m_ScreenX, m_ScreenY);
}
void vga_scrollback(int lines)
{
    for (int y = lines; y < ScreenHeight; y++)
    {
        for (int x = 0; x < ScreenWidth; x++)
        {
            uint8_t color;
            char c = vga_get_cell(x, y, &color);
            vga_set_cell(x, y - lines, c, color);
        }
    }
    for (int y = ScreenHeight - lines; y < ScreenHeight; y++)
    {
        for (int x = 0; x < ScreenWidth; x++)
        {
            vga_set_cell(x, y - lines, ' ', DefaultColor);
        }
    }
        
    m_ScreenY -= lines;
}

void vga_put_char(char c)
{
        if (disableOutput)
    {
        return;
    }
    Outb(0xE9, c);
    switch (c)
    {
        case '\n':
            m_ScreenX = 0;
            m_ScreenY++;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (m_ScreenX % 4); i++)
                vga_put_char(' ');
            break;

        case '\r':
            m_ScreenX = 0;
            break;

        default:
            uint8_t color;
            vga_get_cell(m_ScreenX, m_ScreenY, &color);
            vga_set_cell(m_ScreenX, m_ScreenY, c, color);
            m_ScreenX++;
            break;
    }

    if (m_ScreenX >= ScreenWidth)
    {
        m_ScreenY++;
        m_ScreenX = 0;
    }
    if (m_ScreenY >= ScreenHeight)
        vga_scrollback(1);

    vga_set_cursor(m_ScreenX, m_ScreenY);
}

void putc(char c)
{
    vga_put_char(c);
}

void puts(const char* str)
{
    while(*str)
    {
        putc(*str);
        str++;
    }
}

const char g_HexChars[] = "0123456789abcdef";

void printf_unsigned(unsigned long long number, int radix)
{
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do 
    {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        putc(buffer[pos]);
}

void printf_signed(long long number, int radix)
{
    if (number < 0)
    {
        putc('-');
        printf_unsigned(-number, radix);
    }
    else printf_unsigned(number, radix);
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

int printf(const char* fmt, ...)
{
    const char* pfmt = fmt;
    va_list args;
    va_start(args, fmt);

    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch (state)
        {
            case PRINTF_STATE_NORMAL:
                switch (*fmt)
                {
                    case '%':   state = PRINTF_STATE_LENGTH;
                                break;
                    default:    putc(*fmt);
                                break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                switch (*fmt)
                {
                    case 'h':   length = PRINTF_LENGTH_SHORT;
                                state = PRINTF_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = PRINTF_LENGTH_LONG;
                                state = PRINTF_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (*fmt == 'h')
                {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (*fmt == 'l')
                {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt)
                {
                    case 'c':   putc((char)va_arg(args, int));
                                break;

                    case 's':   
                                puts(va_arg(args, const char*));
                                break;

                    case '%':   putc('%');
                                break;

                    case 'd':
                    case 'i':   radix = 10; sign = true; number = true;
                                break;

                    case 'u':   radix = 10; sign = false; number = true;
                                break;

                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false; number = true;
                                break;

                    case 'o':   radix = 8; sign = false; number = true;
                                break;

                    // ignore invalid spec
                    default:    break;
                }

                if (number)
                {
                    if (sign)
                    {
                        switch (length)
                        {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     printf_signed(va_arg(args, int), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG:        printf_signed(va_arg(args, long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   printf_signed(va_arg(args, int64_t), radix);
                                                        break;
                        }
                    }
                    else
                    {
                        switch (length)
                        {
                        case PRINTF_LENGTH_SHORT_SHORT:
                        case PRINTF_LENGTH_SHORT:
                        case PRINTF_LENGTH_DEFAULT:     printf_unsigned(va_arg(args, unsigned int), radix);
                                                        break;
                                                        
                        case PRINTF_LENGTH_LONG:        printf_unsigned(va_arg(args, unsigned  long), radix);
                                                        break;

                        case PRINTF_LENGTH_LONG_LONG:   printf_unsigned(va_arg(args, uint64_t), radix);
                                                        break;
                        }
                    }
                }

                // reset state
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }

        fmt++;
    }

    va_end(args);
    return (int)fmt-(int)pfmt;
}

void print_buffer(const char* msg, const void* buffer, uint32_t count)
{
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    
    puts(msg);
    for (uint16_t i = 0; i < count; i++)
    {
        putc(g_HexChars[u8Buffer[i] >> 4]);
        putc(g_HexChars[u8Buffer[i] & 0xF]);
    }
    puts("\n");
}

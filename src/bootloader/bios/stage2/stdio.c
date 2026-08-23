/*
 * File: stdio.c
 * File Created: 19 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stdio.h"

#include "io/io.h"

#include <stdarg.h>
#include <stdbool.h>

bool disable_output = false;
uint16_t screen_width = 80;
uint16_t screen_height = 25;
uint8_t default_color = 0x7;
uint8_t* screen_buffer = (uint8_t*)0xB8000;
int cursor_x, cursor_y;

char vga_get_cell(int x, int y, uint8_t *color)
{
    int index = 2 * (y * screen_width + x);
    char c = screen_buffer[index];
    *color = screen_buffer[index + 1];
    return c;
}
void vga_set_cell(int x, int y, char c, uint8_t color)
{
    int index = 2 * (y * screen_width + x);
    screen_buffer[index] = c;
    screen_buffer[index + 1] = color;
}

void vga_set_cursor(int x, int y)
{
    int pos = y * screen_width + x;
    cursor_x = x;
    cursor_y = y;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_get_cursor(int* x, int* y)
{
    *x = cursor_x;
    *y = cursor_y;
}
void vga_clear()
{
    for (int y = 0; y < screen_height; y++)
        for (int x = 0; x < screen_width; x++)
        {
            vga_set_cell(x,y,' ', default_color);
        }
    cursor_x = 0;
    cursor_y = 0;
    vga_set_cursor(cursor_x, cursor_y);
}
void vga_scrollback(int lines)
{
    for (int y = lines; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            uint8_t color;
            char c = vga_get_cell(x, y, &color);
            vga_set_cell(x, y - lines, c, color);
        }
    }
    for (int y = screen_height - lines; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            vga_set_cell(x, y - lines, ' ', default_color);
        }
    }
        
    cursor_y -= lines;
}

void vga_put_char(char c)
{
        if (disable_output)
    {
        return;
    }
    outb(0xE9, c);
    switch (c)
    {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (cursor_x % 4); i++)
                vga_put_char(' ');
            break;

        case '\r':
            cursor_x = 0;
            break;

        default:
            uint8_t color;
            vga_get_cell(cursor_x, cursor_y, &color);
            vga_set_cell(cursor_x, cursor_y, c, color);
            cursor_x++;
            break;
    }

    if (cursor_x >= screen_width)
    {
        cursor_y++;
        cursor_x = 0;
    }
    if (cursor_y >= screen_height)
        vga_scrollback(1);

    vga_set_cursor(cursor_x, cursor_y);
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
    {
        putc(buffer[pos]);
    }
}

void printf_signed(long long number, int radix)
{
    if (number < 0)
    {
        putc('-');
        printf_unsigned(-number, radix);
    }
    else
    {
        printf_unsigned(number, radix);
    }
}

#define PRINTF_STATE_NORMAL       0
#define PRINTF_STATE_LENGTH       1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG  3
#define PRINTF_STATE_SPEC         4

#define PRINTF_LENGTH_DEFAULT     0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT       2
#define PRINTF_LENGTH_LONG        3
#define PRINTF_LENGTH_LONG_LONG   4

int printf(const char *fmt, ...)
{
    const char *pfmt = fmt;
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
            case PRINTF_STATE_NORMAL :
                switch (*fmt)
                {
                    case '%' :
                        state = PRINTF_STATE_LENGTH;
                        break;
                    default :
                        putc(*fmt);
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH :
                switch (*fmt)
                {
                    case 'h' :
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        break;
                    case 'l' :
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        break;
                    default :
                        goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT :
                if (*fmt == 'h')
                {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                }
                else
                {
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_LONG :
                if (*fmt == 'l')
                {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                }
                else
                {
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_SPEC :
PRINTF_STATE_SPEC_:
                switch (*fmt)
                {
                    case 'c' :
                        putc((char)va_arg(args, int));
                        break;

                    case 's' :
                        puts(va_arg(args, const char *));
                        break;

                    case '%' :
                        putc('%');
                        break;

                    case 'd' :
                    case 'i' :
                        radix = 10;
                        sign = true;
                        number = true;
                        break;

                    case 'u' :
                        radix = 10;
                        sign = false;
                        number = true;
                        break;

                    case 'X' :
                    case 'x' :
                    case 'p' :
                        radix = 16;
                        sign = false;
                        number = true;
                        break;

                    case 'o' :
                        radix = 8;
                        sign = false;
                        number = true;
                        break;

                    // ignore invalid spec
                    default :
                        break;
                }

                if (number)
                {
                    if (sign)
                    {
                        switch (length)
                        {
                            case PRINTF_LENGTH_SHORT_SHORT :
                            case PRINTF_LENGTH_SHORT :
                            case PRINTF_LENGTH_DEFAULT :
                                printf_signed(va_arg(args, int), radix);
                                break;

                            case PRINTF_LENGTH_LONG :
                                printf_signed(va_arg(args, long), radix);
                                break;

                            case PRINTF_LENGTH_LONG_LONG :
                                printf_signed(va_arg(args, int64_t), radix);
                                break;
                        }
                    }
                    else
                    {
                        switch (length)
                        {
                            case PRINTF_LENGTH_SHORT_SHORT :
                            case PRINTF_LENGTH_SHORT :
                            case PRINTF_LENGTH_DEFAULT :
                                printf_unsigned(va_arg(args, unsigned int), radix);
                                break;

                            case PRINTF_LENGTH_LONG :
                                printf_unsigned(va_arg(args, unsigned long), radix);
                                break;

                            case PRINTF_LENGTH_LONG_LONG :
                                printf_unsigned(va_arg(args, uint64_t), radix);
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
    return (int)fmt - (int)pfmt;
}

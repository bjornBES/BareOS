/*
 * File: VGATextDevice.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "VGATextDevice.h"
#include "IO.h"
#include "video.h"
#include "string.h"
#include <stddef.h>
#include <stdio.h>

typedef struct
{
    uint8_t extra_glyphs;
    char header[4];
    uint8_t res;
    uint8_t width;
    uint8_t height;
    uint16_t number_of_glyphs;
    uint8_t res2[6];
} PACKED font_struct;

vga_cell_t vga_buffer[60][100];

uint32_t screen_width;
uint32_t screen_height;
uint32_t default_color = 0x00FFFFFF;
int screen_x, screen_y;
uint8_t glyph_width;
uint8_t glyph_height;
uint16_t glyph_number;
uint8_t *font_data;

#define MODULE "VGA"

#define log(...)                           \
    fprintf(VFS_FD_DEBUG, "[%s]", MODULE); \
    fprintf(VFS_FD_DEBUG, __VA_ARGS__);    \
    fprintf(VFS_FD_DEBUG, "\n");

void vga_to_pixel_coords(int cx, int cy, int *px, int *py)
{
    *px = cx * glyph_width;
    *py = cy * glyph_height;
}
void vga_to_cell_coords(int px, int py, int *cx, int *cy)
{
    *cx = px / glyph_width;
    *cy = py / glyph_height;
}

void vga_hexdump(void *ptr, int len)
{
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
            fprintf(VFS_FD_DEBUG, "\n%02x: ", i);
        fprintf(VFS_FD_DEBUG, "%02x ", p[i]);
    }
    fprintf(VFS_FD_DEBUG, "\n");
}

char vga_get_cell(int x, int y, uint32_t *fg_color, uint32_t *bg_color)
{
    if (x < 0 || x >= video_current_mode_data->width || y < 0 || y >= video_current_mode_data->height)
        return 0;

    if (fg_color)
        *fg_color = vga_buffer[y][x].fg_color;
    if (bg_color)
        *bg_color = vga_buffer[y][x].bg_color;

    return vga_buffer[y][x].ch;
}
void vga_set_cell(int x, int y, char c, uint32_t fg_color, uint32_t bg_color)
{
    uint8_t bytes_per_row = (glyph_width + 7) / 8;
    uint16_t bytes_per_glyph = bytes_per_row * glyph_height;

    uint16_t u16_c = (uint16_t)c;
    if (u16_c >= glyph_number)
    {
        log("returning %c(%u) is bigger then %u", c, c, glyph_number);
        return;
    }

    char chr = c;

    int index = ((uint8_t)chr * (bytes_per_glyph));
    uint8_t *glyph = font_data + ((uint8_t)chr * (bytes_per_glyph));
    int px;
    int py;
    vga_to_pixel_coords(x, y, &px, &py);

    for (int row = 0; row < glyph_height; row++)
    {
        uint16_t byte = glyph[row];
        int col = 0;
        //log("%08b %02x", byte, byte);
        for (; col < glyph_width; col++)
        {
            uint16_t offset = (7 - (col % 8));
            // log("offset = %u", offset);
            uint8_t bit = (byte >> offset) & 1;
            if (bit == 1)
            {
                video_set_pixel(px + col, py + row, fg_color);
            }
            else
            {
                video_set_pixel(px + col, py + row, bg_color);
            }
        }
    }

    vga_buffer[y][x].ch = c;
    vga_buffer[y][x].fg_color = fg_color;
    vga_buffer[y][x].bg_color = bg_color;
}

void vga_set_cursor(int x, int y)
{
    int pos = y * screen_width + x;
    screen_x = x;
    screen_y = y;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_get_cursor(int *x, int *y)
{
    *x = screen_x;
    *y = screen_y;
}

void vga_clear()
{
    int save_screen_height = screen_height;
    int save_screen_width = screen_width;
    for (int y = 0; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            vga_set_cell(x, y, ' ', 0, 0);
        }
    }
    screen_x = 0;
    screen_y = 0;
    vga_set_cursor(screen_x, screen_y);
    screen_height = save_screen_height;
    screen_width = save_screen_width;
}

void vga_scrollback(int lines)
{
    for (int y = lines; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            uint32_t fg_color;
            uint32_t bg_color;
            char c = vga_get_cell(x, y, &fg_color, &bg_color);
            vga_set_cell(x, y - lines, c, fg_color, bg_color);
        }
    }
    for (int y = screen_height - lines; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            vga_set_cell(x, y - lines, ' ', default_color, 0);
        }
    }

    screen_y -= lines;
}

void vga_put_char(char c)
{
    switch (c)
    {
    case '\n':
        screen_x = 0;
        screen_y++;
        break;

    case '\t':
        for (int i = 0; i < 4 - (screen_x % 4); i++)
            vga_put_char(' ');
        break;

    case '\r':
        screen_x = 0;
        break;

    default:
        uint32_t color;
        vga_set_cell(screen_x, screen_y, c, default_color, 0);
        screen_x++;
        break;
    }

    if (screen_x >= screen_width)
    {
        screen_y++;
        screen_x = 0;
    }
    if (screen_y >= screen_height)
        vga_scrollback(1);

    vga_set_cursor(screen_x, screen_y);
}

void vga_load_font(void *font)
{
    if (strncmp(font + 1, "F0.1", 4))
    {
        font_struct *font_d = (font_struct *)font;
        glyph_width = font_d->width;
        glyph_height = font_d->height;
        glyph_number = font_d->number_of_glyphs;

        uint16_t number_segments = font_d->extra_glyphs * 16 + 16;
        font_data = font + number_segments;
    }
}

void vga_set_mode(video_mode *mode)
{
    screen_width = mode->width;
    screen_height = mode->height;
}

void vga_init()
{
}

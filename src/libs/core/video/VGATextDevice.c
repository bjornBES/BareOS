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

vga_cell_t vga_buffer[60][100];

uint16_t screen_width;
uint16_t screen_height;
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
    log("bytes_per_glyph = %u bytes_per_row = %u", bytes_per_glyph, bytes_per_row);

    if ((uint8_t)c >= glyph_number)
        return;

    char chr = c - 'A';

    uint8_t *glyph = font_data + ((uint8_t)chr * bytes_per_glyph);
    uint8_t* data = font_data + ((uint8_t)44 * bytes_per_glyph);
    log("&data = %p", data);
    log("&glyph = %p", glyph);
    log("glyph = %X,%X,%X,%X", glyph[0], glyph[1], glyph[2], glyph[3]);
    fprintf(VFS_FD_DEBUG, "char data = [");
    for (size_t i = 0; i < bytes_per_glyph; i++)
    {
        fprintf(VFS_FD_DEBUG, "%X,", data[i]);
    }
    fprintf(VFS_FD_DEBUG, "]\n");

    int px;
    int py;
    vga_to_pixel_coords(x, y, &px, &py);

    for (int row = 0; row < glyph_height; row++)
    {
        uint8_t byte = glyph[row];
        for (int col = 0; col < glyph_width; col++)
        {
            uint8_t bit  = (byte >> (7 - (col % 8))) & 1;

            if (bit == 1)
            {
                video_set_pixel(x + col, y + row, fg_color);
            }
            else
            {
                video_set_pixel(x + col, y + row, bg_color);
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

void vga_get_cursor(int* x, int* y)
{
    *x = screen_x;
    *y = screen_y;
}

void vga_clear()
{
    for (int y = 0; y < screen_height; y++)
    {
        for (int x = 0; x < screen_width; x++)
        {
            vga_set_cell(x,y,' ', 0, 0);
        }
    }
    screen_x = 0;
    screen_y = 0;
    vga_set_cursor(screen_x, screen_y);
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
        uint8_t* u8_font_data = (uint8_t*)font;
        glyph_width = u8_font_data[6];
        glyph_height = u8_font_data[7];
        glyph_number = u8_font_data[8];

        uint16_t number_segments = u8_font_data[0] * 16 + 16;
        font_data = font + number_segments;
    }
}

void vga_set_mode()
{
    screen_width = video_current_mode_data->width;
    screen_height = video_current_mode_data->height;
}

void vga_init()
{

}

/*
 * File: video_text.c
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "video.h"
#include "video_types.h"
#include "video_common.h"
#include "video_color.h"

#include "kernel/string.h"
#include "kernel/memory.h"

#include "device/device.h"
#include <defs.h>

#define MODULE "VGA"

typedef struct font_struct
{
    uint8_t extra_glyphs;
    char header[4];
    uint8_t res;
    uint8_t width;
    uint8_t height;
    uint16_t number_of_glyphs;
    uint8_t res2[6];
} PACKED font_struct_t;

typedef struct font_data
{
    uint16_t glyph_width;
    uint16_t glyph_height;
    uint32_t glyph_number;
    uint8_t *data;
} font_data_t;

font_data_t *font_data;

static inline void video_text_to_pixel_coords(int cx, int cy, int *px, int *py)
{
    *px = cx * font_data->glyph_width;
    *py = cy * font_data->glyph_height;
}

static inline void video_text_to_cell_coords(int px, int py, int *cx, int *cy)
{
    *cx = px / font_data->glyph_width;
    *cy = py / font_data->glyph_height;
}

void video_text_load_font(fd_t file)
{
    
}
void video_text_load_font_from_data(void *font)
{
    if (font_data == NULL)
    {
        font_data = malloc(sizeof(font_data_t));
    }
    uint8_t *font_file = (uint8_t *)font;
    log_debug(MODULE, "font header %s", font_file + 1);
    if (!strncmp(font + 1, "F0.1", 4))
    {
        font_struct_t *font_d = (font_struct_t *)font_file;
        log_debug(MODULE, "font data %x, %s, %u, %u, %u, %u", font_d->extra_glyphs, font_d->header, font_d->res, font_d->width, font_d->height, font_d->number_of_glyphs);
        font_data->glyph_width = font_d->width;
        font_data->glyph_height = font_d->height;
        font_data->glyph_number = font_d->number_of_glyphs;
        vaddr_t number_segments = font_d->extra_glyphs * 16 + 16;
        font_data->data = font_file + number_segments;
    }
}

void video_text_set_cell(device_t *video_driver, int x, int y, char c, uint32_t fg_color, uint32_t bg_color)
{
    ENTER_FUNC(MODULE, "%p, %u, %u, '%c'(%u), %x, %x", video_driver, x, y, c, c, fg_color, bg_color);
    video_priv_t *video_priv = (video_priv_t *)video_driver->priv;
    uint8_t bytes_per_row = (font_data->glyph_width + 7) / 8;
    uint16_t bytes_per_glyph = bytes_per_row * font_data->glyph_height;

    uint16_t u16_c = (uint16_t)c;
    if (u16_c >= font_data->glyph_number)
    {
        log_debug(MODULE, "returning %c(%u) is bigger then %u", c, c, font_data->glyph_number);
        return;
    }

    char chr = c;

    uint8_t *glyph = font_data->data + ((uint8_t)chr * (bytes_per_glyph));
    int px;
    int py;
    video_text_to_pixel_coords(x, y, &px, &py);

    uint8_t fg_red = 0;
    uint8_t fg_green = 0;
    uint8_t fg_blue = 0;
    uint8_t bg_red = 0;
    uint8_t bg_green = 0;
    uint8_t bg_blue = 0;

    video_unpack_color(&video_priv->fb, fg_color, &fg_red, &fg_green, &fg_blue);
    video_unpack_color(&video_priv->fb, bg_color, &bg_red, &bg_green, &bg_blue);

    uint8_t bytes_pp = video_priv->fb.bpp / 8;
    uint8_t *buffer = malloc(font_data->glyph_width * bytes_pp); 
    for (int row = 0; row < font_data->glyph_height; row++)
    {
        uint16_t byte = glyph[row];
        int col = 0;
        // log_debug(MODULE, "%08b %02x", byte, byte);
        for (; col < font_data->glyph_width; col++)
        {
            int buffer_index = col * bytes_pp;
            uint16_t offset = (7 - (col % 8));
            // log_debug(MODULE, "offset = %u", offset);
            uint8_t bit = (byte >> offset) & 1;
            if (bit == 1)
            {
                video_pack_to_buffer(&video_priv->fb, (uint8_t*)(buffer + buffer_index), fg_red, fg_green, fg_blue);
            }
            else
            {
                video_pack_to_buffer(&video_priv->fb, (uint8_t*)(buffer + buffer_index), bg_red, bg_green, bg_blue);
            }
        }
        int index = ((py + row) * video_priv->fb.width + px) * bytes_pp;
        device_write(video_driver, (void*)buffer, index, font_data->glyph_width * bytes_pp);
    }
    free(buffer);
}

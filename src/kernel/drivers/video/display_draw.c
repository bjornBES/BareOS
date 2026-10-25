/*
 * File: display_draw.c
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/display.h"

#include "debug/debug.h"

#include "memory.h"

#define MODULE "display_draw"

INTERNAL void display_draw_unpack(const video_framebuffer_t *fb, color_t packed_color, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = (uint8_t)(packed_color >> (8 - fb->r_size)) & 0xFF;
    *g = (uint8_t)(packed_color >> (8 - fb->g_size)) & 0xFF;
    *b = (uint8_t)(packed_color >> (8 - fb->b_size)) & 0xFF;
    return;
}

INTERNAL void display_draw_unpack_fbcon(const video_framebuffer_t *fb, color_t packed_color, uint8_t *r, uint8_t *g, uint8_t *b)
{
    // 0x00_RR_GG_BB
    *b = (uint8_t)(packed_color) & 0xFF;
    *g = (uint8_t)(packed_color >> 8) & 0xFF;
    *r = (uint8_t)(packed_color >> 16) & 0xFF;
    return;
}

// Pack r/g/b into a pixel for this surface, using its channel masks.
// Never hardcode channel order; always go through this.
INTERNAL color_t display_draw_pack(const video_framebuffer_t *fb, uint8_t r, uint8_t g, uint8_t b)
{
    color_t color = (r << fb->r_pos) | (g << fb->g_pos) | (b << fb->b_pos);
    return color;
}

INTERNAL void display_draw_pack_to_buffer(const video_framebuffer_t *fb, uint8_t *buffer, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = (r << fb->r_pos) | (g << fb->g_pos) | (b << fb->b_pos);
    memcpy(buffer, &color, fb->bpp / 8);
}

ALWAYS_INLINE uint32_t display_draw_get_index(const video_framebuffer_t *fb, uint32_t x, uint32_t y)
{
    return y * fb->pitch + x * (fb->bpp / 8);
}

ALWAYS_INLINE uint8_t *display_draw_get_address(const video_framebuffer_t *fb, uint32_t x, uint32_t y)
{
    return (uint8_t *)fb->addr + y * fb->pitch + x * (fb->bpp / 8);
}

// All drawing functions clip to the surface. Nothing is written outside it.
status_t display_draw_putpixel(const video_framebuffer_t *fb, uint32_t x, uint32_t y, color_t color)
{
    uint8_t color_red = 0;
    uint8_t color_green = 0;
    uint8_t color_blue = 0;
    display_draw_unpack_fbcon(fb, color, &color_red, &color_green, &color_blue);

    uint8_t *pixel = display_draw_get_address(fb, x, y);
    display_draw_pack_to_buffer(fb, pixel, color_red, color_green, color_blue);
    return KERRNO_SUCCESSES;
}

status_t display_draw_fillrect(const video_framebuffer_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color)
{
    uint8_t color_red = 0;
    uint8_t color_green = 0;
    uint8_t color_blue = 0;
    display_draw_unpack_fbcon(fb, color, &color_red, &color_green, &color_blue);

    for (uint32_t row = y; row < y + h; row++)
    {
        uint8_t *line = display_draw_get_address(fb, x, row);
        display_draw_pack_to_buffer(fb, line, color_red, color_green, color_blue);
    }
    return KERRNO_SUCCESSES;
}

status_t display_draw_clear(const video_framebuffer_t *fb, color_t color)
{
    return display_draw_fillrect(fb, 0, 0, fb->width, fb->height, color);
}

status_t display_draw_rect(const video_framebuffer_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color); // outline only

// Copy a pixel buffer onto the surface (for example a RAM back buffer).
// src must have the same pixel format as dst; src_pitch is in bytes.
status_t display_draw_blit(const video_framebuffer_t *dst, uint32_t dx, uint32_t dy, const void *src, uint32_t src_pitch, uint32_t w, uint32_t h);

// Draw a 1bpp bitmap (a font glyph): set bits get fg, clear bits get bg.
// Rows are (w + 7) / 8 bytes, most significant bit first (the PSF layout).
status_t display_draw_blit_mono(const video_framebuffer_t *dst, uint32_t x, uint32_t y, const uint8_t *bitmap, uint32_t w, uint32_t h, color_t fg, color_t bg)
{
    // ENTER_FUNC("%p, %u, %u, %p, %u, %u, 0x%08x, 0x%08x", dst, x, y, bitmap, w, h, fg, bg);
    uint8_t fg_red = 0;
    uint8_t fg_green = 0;
    uint8_t fg_blue = 0;

    uint8_t bg_red = 0;
    uint8_t bg_green = 0;
    uint8_t bg_blue = 0;

    display_draw_unpack_fbcon(dst, fg, &fg_red, &fg_green, &fg_blue);
    display_draw_unpack_fbcon(dst, bg, &bg_red, &bg_green, &bg_blue);

    int row_size = (w + 7) / 8;
    uint8_t *buffer = kzmalloc(row_size * (dst->bpp / 8));

    for (int row = 0; row < h; row++)
    {
        int col = 0;
        for (; col < w; col++)
        {
            int buffer_index = col * (dst->bpp / 8);
            int bitmap_index = row * w + col;
            uint8_t byte = bitmap[row];

            uint16_t offset = (7 - (col % 8));
            uint8_t bit = (byte >> offset) & 1;

            if (bit == 1)
            {
                display_draw_pack_to_buffer(dst, (uint8_t*)(buffer + buffer_index), fg_red, fg_green, fg_blue);
            }
            else
            {
                display_draw_pack_to_buffer(dst, (uint8_t*)(buffer + buffer_index), bg_red, bg_green, bg_blue);
            }
        }
        uint32_t *pixel = (uint32_t *)(display_draw_get_address(dst, x, y + row));
        // trace_debug(MODULE, "%08b 0x%x %p, %p", bitmap[row], row, &bitmap[row], pixel);
        memcpy(pixel, buffer, w * (dst->bpp / 8));
    }
    kfree(buffer);

    return KERRNO_SUCCESSES;
}

// Move a rectangle within the same surface (safe when the areas overlap).
// These READ the framebuffer, which is very slow, so prefer redrawing from
// the console's character grid or a back buffer.
status_t display_draw_copyrect(const video_framebuffer_t *fb, uint32_t dx, uint32_t dy, uint32_t sx, uint32_t sy, uint32_t w, uint32_t h);
status_t display_draw_scroll_up(const video_framebuffer_t *fb, uint32_t lines, color_t fill_color);

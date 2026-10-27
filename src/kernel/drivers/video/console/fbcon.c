/*
 * File: fbcon.c
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/console/fbcon.h"
#include "drivers/video/console/font_psf.h"

#include "drivers/video/display.h"

#include "init.h"

#include "memory.h"

#include "kerrno.h"

#define MODULE "fbcon"

const font_t *active_font;
const video_framebuffer_t *active_fb;

fbcon_cell_t *text_buffer;

int screen_x;
int screen_y;
fbcon_size_t size;

color_t fg_color;
color_t bg_color;

// copies the overlap
INTERNAL status_t fbcon_resize_grid(uint32_t cols, uint32_t rows)
{
    if (size.cols == cols && size.rows == rows && text_buffer != NULL)
    {
        return KERRNO_SUCCESSES;
    }

    if (text_buffer == NULL)
    {
        text_buffer = kzmalloc((cols * rows) * sizeof(fbcon_cell_t));
    }
    else
    {
        text_buffer = krealloc(text_buffer, (cols * rows) * sizeof(fbcon_cell_t));
    }

    size.cols = cols;
    size.rows = rows;
    return KERRNO_SUCCESSES;
}

// recompute cols/rows
INTERNAL status_t fbcon_bind_surface(const video_framebuffer_t *fb)
{
    active_fb = fb;
    return fbcon_resize_grid(fb->width / active_font->width, fb->height / active_font->height);
}

// display_listener_t: primary changed or its mode changed.
INTERNAL status_t fbcon_on_display_change(display_device_t *primary, void *ctx)
{
    ENTER_FUNC("%p, %p", primary, ctx);
    video_framebuffer_t *fb = &primary->frame_buffer;
    return fbcon_bind_surface(fb);
    // return fbcon_redraw();
}

// Load the font, allocate the character grid, register the display listener
// and the kernel log sink, and bind to the current primary (if any).
// With no primary yet it starts buffer-only and draws once one registers.

status_t fbcon_init()
{
    display_add_listener(fbcon_on_display_change, NULL);
    active_font = psf_font_builtin();
    screen_x = 0;
    screen_y = 0;
    size.cols = 0;
    size.rows = 0;
    fg_color = 0x00FFFFFF;
    bg_color = 0x00000000;
    return KERRNO_SUCCESSES;
}

SUBSYS_INITCALL(fbcon_init);

INTERNAL status_t fbcon_draw_cell(uint32_t col, uint32_t row)
{
    fbcon_cell_t *call = &text_buffer[row * size.cols + col];
    uint8_t *glyph = NULL;
    font_get_glyph(active_font, call->cp, &glyph);
    return display_draw_blit_mono(active_fb, col * active_font->width, row * active_font->height, glyph, active_font->width, active_font->height, call->fg, call->bg);
}

// moves grid rows, then redraws
INTERNAL status_t fbcon_scroll(uint32_t lines)
{
    ENTER_FUNC("%u", lines);
    uint32_t save_x = screen_x;
    uint32_t save_y = screen_y;

    fbcon_cell_t *src = &text_buffer[lines * size.cols];
    fbcon_cell_t *dst = &text_buffer[0];
    size_t _size = size.cols * (size.rows - lines);
    fbcon_cell_t *new = &text_buffer[_size];
    memmove(dst, src, _size * sizeof(fbcon_cell_t));
    // lines = 5
    // size.cols = 80
    // size.rows = 80
    // src_index = 5 * 80 = 400
    // src = &text_buffer[5 * 80] = &text_buffer[400]
    // dst_index = 0
    // dst = &text_buffer[0] = &text_buffer
    // size = 80 * (80 - 5) = 6000
    // max = 80 * 80 = 6400
    memset(new, 0, (lines * size.cols) * sizeof(fbcon_cell_t));

    screen_y -= lines;
    return fbcon_redraw_region(0, save_y, size.cols - 1, screen_y);
}

// advances and scrolls at the bottom
INTERNAL status_t fbcon_newline()
{
    ENTER_FUNC("", 0);
    screen_x = 0;
    screen_y++;

    if (screen_y >= size.rows)
    {
        return fbcon_scroll(1);
    }
    return KERRNO_SUCCESSES;
}

void fbcon_put_char(uint32_t cp)
{
    // ENTER_FUNC("%x ('%c')", cp, cp);
    switch (cp)
    {
        case '\n' :
            fbcon_newline();
            break;

        case '\t' :
            {
                uint32_t out_cp;
                font_glyph_index(active_font, ' ', &out_cp);
                for (int i = 0; i < 4 - (screen_x % 4); i++)
                {
                    fbcon_cell_t *call = &text_buffer[screen_y * size.cols + screen_x];
                    call->cp = out_cp;
                    call->fg = fg_color;
                    call->bg = bg_color;
                    screen_x++;
                }
            }
            break;

        case '\r' :
            screen_x = 0;
            break;

        case '\b' :
            screen_x--;
            if (screen_x == 0)
            {
                screen_x = size.cols - 1;
                screen_y--;
            }
            break;

        default :
            {
                fbcon_cell_t *call = &text_buffer[screen_y * size.cols + screen_x];
                font_glyph_index(active_font, cp, &call->cp);
                call->fg = fg_color;
                call->bg = bg_color;
                // fbcon_draw_cell(screen_x, screen_y);
                screen_x++;
            }
            break;
    }

    if (screen_x >= size.cols)
    {
        screen_y++;
        screen_x = 0;
    }

    if (screen_y >= size.rows)
    {
        fbcon_scroll(1);
        screen_x--;
        fbcon_put_char(cp);
    }
}

// handles \n \r \t \b and wrapping
status_t fbcon_putc(uint32_t cp)
{
    // ENTER_FUNC("0x%x", cp);
    uint32_t save_x = screen_x;
    uint32_t save_y = screen_y;
    fbcon_put_char(cp);
    return fbcon_redraw_region(save_x, save_y, screen_x, screen_y);
}

status_t fbcon_write(const char *s, size_t len)
{
    uint32_t save_x = screen_x;
    uint32_t save_y = screen_y;
    for (size_t i = 0; i < len; i++)
    {
        fbcon_put_char(s[i]);
    }
    return fbcon_redraw_region(save_x, save_y, screen_x, screen_y);
}

// clears grid and screen, cursor to 0,0
status_t fbcon_clear()
{
    memset(text_buffer, 0, size.rows * size.cols * sizeof(fbcon_cell_t));
    fbcon_set_cursor(0, 0);
    return fbcon_redraw();
}

// applies to following output
status_t fbcon_set_color(color_t fg, color_t bg)
{
    fg_color = fg;
    bg_color = bg;
    return KERRNO_SUCCESSES;
}

status_t fbcon_get_size(fbcon_size_t *out)
{
    *out = size;
    return KERRNO_SUCCESSES;
}

status_t fbcon_set_cursor(uint32_t col, uint32_t row)
{
    screen_x = col;
    screen_y = row;
    return KERRNO_SUCCESSES;
}

status_t fbcon_get_cursor(uint32_t *col, uint32_t *row)
{
    *col = screen_x;
    *row = screen_y;
    return KERRNO_SUCCESSES;
}

status_t fbcon_show_cursor(int visible);

// whole grid onto the current surface
status_t fbcon_redraw()
{
    for (size_t row = 0; row < size.rows; row++)
    {
        for (size_t col = 0; col < size.cols; col++)
        {
            fbcon_cell_t *call = &text_buffer[row * size.cols + col];
            uint8_t *glyph = NULL;
            font_get_glyph(active_font, call->cp, &glyph);
            display_draw_blit_mono(active_fb, col * active_font->width, row * active_font->height, glyph, active_font->width, active_font->height, call->fg, call->bg);
        }
    }
    return KERRNO_SUCCESSES;
}

status_t fbcon_redraw_region(uint32_t start_col, uint32_t start_row, uint32_t cols, uint32_t rows)
{
    // ENTER_FUNC("%u, %u, %u, %u", start_col, start_row, cols, rows);
    if (start_row == rows)
    {
        for (size_t col = start_col; col < cols; col++)
        {
            fbcon_cell_t *call = &text_buffer[start_row * size.cols + col];
            uint8_t *glyph = NULL;
            font_get_glyph(active_font, call->cp, &glyph);
            display_draw_blit_mono(active_fb, col * active_font->width, start_row * active_font->height, glyph, active_font->width, active_font->height, call->fg, call->bg);
        }
        return KERRNO_SUCCESSES;
    }
    for (size_t row = start_row; row < rows; row++)
    {
        for (size_t col = start_col; col < cols; col++)
        {
            fbcon_cell_t *call = &text_buffer[row * size.cols + col];
            uint8_t *glyph = NULL;
            font_get_glyph(active_font, call->cp, &glyph);
            display_draw_blit_mono(active_fb, col * active_font->width, row * active_font->height, glyph, active_font->width, active_font->height, call->fg, call->bg);
        }
    }
    return KERRNO_SUCCESSES;
}

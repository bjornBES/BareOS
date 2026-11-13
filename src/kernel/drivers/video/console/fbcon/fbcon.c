/*
 * File: fbcon.c
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/console/fbcon/fbcon.h"
#include "drivers/video/console/font_psf.h"

#include "drivers/video/display.h"

#include "drivers/tty/vt/vt.h"

#include "init.h"

#include "memory.h"

#include "string.h"

#include "kerrno.h"

#include <array.h>

#define FB_MAX 32

#define MODULE "fbcon"

static unsigned int first_fb_vc = 0;
static unsigned int last_fb_vc = MAX_CONSOLES - 1;

static display_device_t *fb_display[MAX_CONSOLES];

static video_framebuffer_t *fbcon_registered_fb[FB_MAX];
static int fbcon_num_registered_fb;

static int con2fb_map[MAX_CONSOLES];

font_t *default_font;

static video_framebuffer_t *fbcon_info_from_console(int console)
{
    // int fb = con2fb_map[console];
    // if (fb < 0 || fb >= ARRAY_SIZE(fbcon_registered_fb))
    // {
    //     return NULL;
    // }

    return fbcon_registered_fb[console];
}

// copies the overlap
INTERNAL status_t fbcon_resize_grid(uint32_t cols, uint32_t rows)
{
    ENTER_FUNC("%u, %u", cols, rows);
    // if (size.cols == cols && size.rows == rows && text_buffer != NULL)
    // {
    //     return KERRNO_SUCCESSES;
    // }

    // if (text_buffer == NULL)
    // {
    //     text_buffer = kzmalloc((cols * rows) * sizeof(fbcon_cell_t));
    // }
    // else
    // {
    //     text_buffer = krealloc(text_buffer, (cols * rows) * sizeof(fbcon_cell_t));
    // }

    // size.cols = cols;
    // size.rows = rows;
    return KERRNO_SUCCESSES;
}

// recompute cols/rows
INTERNAL status_t fbcon_bind_surface(const video_framebuffer_t *fb)
{
    ENTER_FUNC("%p", fb);
    // active_fb = fb;
    // return fbcon_resize_grid(fb->width / vc->current_font->width, fb->height / vc->current_font->height);
    return KERRNO_UNSUCCESS;
}

// display_listener_t: primary changed or its mode changed.
INTERNAL status_t fbcon_on_display_change(display_device_t *primary_display_dev, void *ctx)
{
    ENTER_FUNC("%p, %p", primary_display_dev, ctx);
    vc_data_t *vc = ctx;

    trace_info(MODULE, "primary_display_dev->font_data = %p", primary_display_dev->font_data);

    video_framebuffer_t *fb = &primary_display_dev->frame_buffer;
    fbcon_registered_fb[vc->vc_index] = fb;
    fb_display[vc->vc_index] = primary_display_dev;
    if (!primary_display_dev->font_data)
    {
        primary_display_dev->font_data = default_font;
    }
    vc_resize(vc, fb->width / primary_display_dev->font_data->width, fb->height / primary_display_dev->font_data->height, false);
    // return fbcon_bind_surface(fb);
    // return fbcon_redraw();
    return KERRNO_SUCCESSES;
}

INTERNAL INLINE bool fbcon_is_active(vc_data_t *vc, video_framebuffer_t *info)
{
    return vc_use_text(vc);
}

INTERNAL status_t fbcon_draw_cell(vc_data_t *vc, video_framebuffer_t *info, vt_cell_t *call, uint32_t col, uint32_t row)
{
    // ENTER_FUNC("%p, %p, %p, %u, %u", vc, info, call, col, row);
    uint8_t *glyph = NULL;
    display_device_t *display_dev = fb_display[vc->vc_index];
    if (!display_dev)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "");
    }
    font_t *font = display_dev->font_data;
    font_get_glyph(font, call->cp, &glyph);
    return display_draw_blit_mono(info, col * font->width, row * font->height, glyph, font->width, font->height, call->fg, call->bg);
}

// moves grid rows, then redraws
INTERNAL status_t fbcon_scroll(vc_data_t *vc, uint32_t lines)
{
    ENTER_FUNC("%p, %u", vc, lines);

    // fbcon_cell_t *src = &vc->cell_buffer[lines * vc->vc_cols];
    // fbcon_cell_t *dst = &vc->cell_buffer[0];
    // size_t _size = vc->vc_cols * (vc->vc_rows - lines);
    // fbcon_cell_t *new = &vc->cell_buffer[_size];
    // memmove(dst, src, _size * sizeof(fbcon_cell_t));

    // memset(new, 0, (lines * vc->vc_cols) * sizeof(fbcon_cell_t));

    // screen_y -= lines;
    // return fbcon_redraw_region(0, save_y, size.cols - 1, screen_y);
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

// clears grid and screen, cursor to 0,0
status_t fbcon_clear()
{
    return fbcon_redraw();
}

// whole grid onto the current surface
status_t fbcon_redraw()
{
    // for (size_t row = 0; row < size.rows; row++)
    // {
    //     for (size_t col = 0; col < size.cols; col++)
    //     {
    //         fbcon_cell_t *call = &text_buffer[row * size.cols + col];
    //         uint8_t *glyph = NULL;
    //         font_get_glyph(vc->current_font, call->cp, &glyph);
    //         display_draw_blit_mono(active_fb, col * vc->current_font->width, row * vc->current_font->height, glyph, vc->current_font->width, vc->current_font->height, call->fg, call->bg);
    //     }
    // }
    return KERRNO_SUCCESSES;
}

status_t fbcon_redraw_region(uint32_t start_col, uint32_t start_row, uint32_t cols, uint32_t rows)
{
    // ENTER_FUNC("%u, %u, %u, %u", start_col, start_row, cols, rows);
    // if (start_row == rows)
    // {
    //     for (size_t col = start_col; col < cols; col++)
    //     {
    //         fbcon_cell_t *call = &text_buffer[start_row * size.cols + col];
    //         uint8_t *glyph = NULL;
    //         font_get_glyph(vc->current_font, call->cp, &glyph);
    //         display_draw_blit_mono(active_fb, col * vc->current_font->width, start_row * vc->current_font->height, glyph, vc->current_font->width, vc->current_font->height, call->fg, call->bg);
    //     }
    //     return KERRNO_SUCCESSES;
    // }
    // for (size_t row = start_row; row < rows; row++)
    // {
    //     for (size_t col = start_col; col < cols; col++)
    //     {
    //         fbcon_cell_t *call = &text_buffer[row * size.cols + col];
    //         uint8_t *glyph = NULL;
    //         font_get_glyph(vc->current_font, call->cp, &glyph);
    //         display_draw_blit_mono(active_fb, col * vc->current_font->width, row * vc->current_font->height, glyph, vc->current_font->width, vc->current_font->height, call->fg, call->bg);
    //     }
    // }
    return KERRNO_SUCCESSES;
}

status_t fbcon_resize(vc_data_t *vc, uint32_t width, uint32_t height, bool from_user)
{
    return KERRNO_SUCCESSES;
}

status_t fbcon_putcs(vc_data_t *vc, const vt_cell_t *s, size_t count, uint32_t xpos, uint32_t ypos)
{
    ENTER_FUNC("%p, %p, %u, %u, %u", vc, s, count, xpos, ypos);
    video_framebuffer_t *info = fbcon_info_from_console(vc->vc_index);
    display_device_t *p = fb_display[vc->vc_index];
    // struct fbcon_par *par = info->fbcon_par;

    if (fbcon_is_active(vc, info))
    {
        while (count)
        {
            fbcon_draw_cell(vc, info, (vt_cell_t *)s, xpos, ypos);
            s++;
            count--;
            xpos++;
        }
    }
    return KERRNO_UNSUCCESS;
}

status_t fbcon_putc(vc_data_t *vc, vt_cell_t c, uint32_t x, uint32_t y)
{
    // ENTER_FUNC("0x%x", cp);
    // uint32_t save_x = screen_x;
    // uint32_t save_y = screen_y;
    // fbcon_put_char(ch);
    // return fbcon_redraw_region(save_x, save_y, screen_x, screen_y);
    return KERRNO_UNSUCCESS;
}

status_t fbcon_switch(vc_data_t *vc)
{
    ENTER_FUNC("%p", vc);
    video_framebuffer_t *info = fbcon_info_from_console(vc->vc_index);
    display_device_t *p = fb_display[vc->vc_index];
    return KERRNO_SUCCESSES;
}

status_t fbcon_init(vc_data_t *vc, bool do_init)
{
    ENTER_FUNC("%p, %s", vc, do_init BOOL_TO_STRING);
    if (do_init)
    {
        display_add_listener(fbcon_on_display_change, vc);
        fbcon_registered_fb[vc->vc_index] = NULL;
    }

    display_device_t *display_dev = display_get_primary();
    trace_info(MODULE, "display_dev = %p", display_dev);
    if (display_dev == NULL)
    {
        vc->vc_cols = 80;
        vc->vc_rows = 40;
    }
    return KERRNO_SUCCESSES;
}

const virt_con_ops_t fbcon = {
    .con_name = "frame buffer device",
    .con_flags = BIT(VC_FLAGS_UTF8) | BIT(VC_FLAGS_COLOR) | BIT(VC_FLAGS_TEXT),
    .con_init = fbcon_init,

    .con_switch = fbcon_switch,

    .con_putcs = fbcon_putcs,
    .con_putc = fbcon_putc,
};

// Load the font, allocate the character grid, register the display listener
// and the kernel log sink, and bind to the current primary (if any).
// With no primary yet it starts buffer-only and draws once one registers.

status_t fbcon_initcall()
{
    ENTER_FUNC("", 0);
    default_font = (font_t *)psf_font_builtin();
    vc_take_over_console(&fbcon, first_fb_vc, last_fb_vc, true);
    // return display_add_listener(fbcon_on_display_change, NULL);
    return KERRNO_SUCCESSES;
}

SUBSYS_INITCALL(fbcon_initcall)

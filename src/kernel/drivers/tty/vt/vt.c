/*
 * File: vt.c
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/vt/vt.h"
#include "drivers/tty/vt/vtt.h"
#include "drivers/tty/vt/vt_buffer.h"

#include "drivers/tty/tty.h"
#include "drivers/base/class.h"
#include "drivers/base/core.h"

#include "drivers/video/console/font_psf.h"

#include "console/console.h"
#include "console/consolet.h"

#include "mm/pmm/pmm.h"

#include "memory.h"
#include "math.h"

#include "string.h"

#include "kerrno.h"
#include "init.h"

#define MODULE                 "vt"

#define update_screen(x)       vc_redraw_screen(x, 0)
#define switch_screen(x)       vc_redraw_screen(x, 1)

#define CON_DRIVER_FLAG_MODULE 1
#define CON_DRIVER_FLAG_INIT   2
#define CON_DRIVER_FLAG_ATTR   4
#define CON_DRIVER_FLAG_ZOMBIE 8

typedef struct console_driver
{
    const virt_con_ops_t *con;
    const char *desc;
    device_t *dev;
    int node;
    int first;
    int last;
    int flag;
} console_driver_t;

static console_driver_t registered_con_driver[MAX_CONSOLE_DRIVER];

static virt_con_ops_t *con_driver_map[MAX_CONSOLES];
static virtual_console_t consoles[MAX_CONSOLES];

virt_con_ops_t *conswitchp;
extern const virt_con_ops_t dummy_con;

static vc_data_t *master_display_fg;

static int fg_console;

#define LOCK_NAME vt_lock
DEF_LOCK(LOCK_NAME);

color_t default_color_table[] = {
    TO_RGBX(0, 0, 0, 0),
    TO_RGBX(0xAA, 0, 0, 0),
    TO_RGBX(0, 0xAA, 0, 0),
    TO_RGBX(0xAA, 0x55, 0, 0),
    TO_RGBX(0, 0, 0xAA, 0),
    TO_RGBX(0xAA, 0, 0xAA, 0),
    TO_RGBX(0, 0xAA, 0xAA, 0),
    TO_RGBX(0xAA, 0xAA, 0xAA, 0),
    TO_RGBX(0x55, 0x55, 0x55, 0),
    TO_RGBX(0xFF, 0x55, 0x55, 0),
    TO_RGBX(0x55, 0xFF, 0x55, 0),
    TO_RGBX(0xFF, 0xFF, 0x55, 0),
    TO_RGBX(0x55, 0x55, 0xFF, 0),
    TO_RGBX(0xFF, 0x55, 0xFF, 0),
    TO_RGBX(0x55, 0xFF, 0xFF, 0),
    TO_RGBX(0xFF, 0xFF, 0xFF, 0),
};

enum vc_ctl_state
{
    ESnormal,
    ESesc,
    ESsquare,
    ESgetpars,
    ESgetsubpars,
    ESfunckey,
    EShash,
    ESsetG0,
    ESsetG1,
    ESpercent,
    EScsiignore,
    ESnonstd,
    ESpalette,
    ESosc,
    ESANSI_first = ESosc,
    ESapc,
    ESpm,
    ESdcs,
    ESANSI_last = ESdcs,
};

enum
{
    ASCII_NULL = 0,
    ASCII_BELL = 7,
    ASCII_BACKSPACE = 8,
    ASCII_IGNORE_FIRST = ASCII_BACKSPACE,
    ASCII_HTAB = 9,
    ASCII_LINEFEED = 10,
    ASCII_VTAB = 11,
    ASCII_FORMFEED = 12,
    ASCII_CAR_RET = 13,
    ASCII_IGNORE_LAST = ASCII_CAR_RET,
    ASCII_SHIFTOUT = 14,
    ASCII_SHIFTIN = 15,
    ASCII_CANCEL = 24,
    ASCII_SUBSTITUTE = 26,
    ASCII_ESCAPE = 27,
    ASCII_CSI_IGNORE_FIRST = ' ', /* 0x2x, 0x3a and 0x3c - 0x3f */
    ASCII_CSI_IGNORE_LAST = '?',
    ASCII_DEL = 127,
    ASCII_EXT_CSI = 128 + ASCII_ESCAPE,
};

INTERNAL_INLINE vt_cell_t *vc_cell(vc_data_t *vc, vt_cell_t *buf)
{
    return &buf[(size_t)vc->vc_y * vc->vc_cols + vc->vc_x];
}

INTERNAL_INLINE vt_cell_t *vc_buffer_cell(vc_data_t *vc, uint16_t x, uint16_t y)
{
    return &vc->vc_screenbuf[(size_t)y * vc->vc_cols + x];
}

INTERNAL_INLINE vt_cell_t *vc_origin_cell(vc_data_t *vc, uint16_t x, uint16_t y)
{
    return &vc->vc_origin[(size_t)y * vc->vc_cols + x];
}

INTERNAL status_t vc_data_init(vc_data_t *vc, int num, bool init)
{
    ENTER_FUNC("%p, %u, %s", vc, num, init BOOL_TO_STRING);
    trace_debug(MODULE, "conswitchp = %p", conswitchp);
    vc->vc_sw = conswitchp;

    if (con_driver_map[num])
    {
        vc->vc_sw = con_driver_map[num];
    }

    vc->vc_index = num;
    vc->vc_display_fg = &master_display_fg;
    // vc->can_do_color = 0;
    status_t ret = vc->vc_sw->con_init(vc, init);
    if (ret == KERRNO_SUCCESSES)
    {
        // vc->vc_size_row = vc->vc_cols << 1;
        vc->vc_screenbuf_cells = vc->vc_rows * vc->vc_cols;
        return KERRNO_SUCCESSES;
    }
    return ret;
}

INTERNAL void vt_data_deinit(vc_data_t *vc)
{
    vc->vc_sw->con_deinit(vc);
}

bool con_is_visible(const vc_data_t *vc)
{
    // trace_debug(MODULE, "*vc->vc_display_fg = %p, vc = %p", *vc->vc_display_fg, vc);
    WARN_LOCK_UNLOCKED();
    return *vc->vc_display_fg == vc;
}

INTERNAL_INLINE bool vc_is_fg(vc_data_t *vc)
{
    return vc->vc_index == fg_console;
}

INTERNAL_INLINE bool con_should_update(const vc_data_t *vc)
{
    return con_is_visible(vc);
}

INTERNAL_INLINE vt_cell_t *screenpos(const struct vc_data *vc, vaddr_t offset, bool viewed)
{
    vt_cell_t *origin = viewed ? vc->vc_visible_origin : vc->vc_origin;

    return (vt_cell_t *)(origin + offset);
}

INTERNAL status_t vc_putc(vc_data_t *vc, vt_cell_t *ca, uint32_t y, uint32_t x)
{
    if (vc->vc_sw->con_putc)
    {
        return vc->vc_sw->con_putc(vc, *ca, x, y);
    }
    else
    {
        return vc->vc_sw->con_putcs(vc, ca, 1, x, y);
    }
}

INTERNAL void vc_do_update_region(vc_data_t *vc, vt_cell_t *start, size_t count)
{
    ENTER_FUNC("%p, %p, 0x%x", vc, start, count);
    unsigned int xx, yy, offset;
    vt_cell_t *p = (vt_cell_t *)start;

    offset = ((vaddr_t)start - (vaddr_t)vc->vc_origin);
    xx = offset % vc->vc_cols;
    yy = offset / vc->vc_cols;

    for (;;)
    {
        uint32_t attrib = scr_readw(p).attr;
        int startx = xx;
        vt_cell_t *q = p;
        while (xx < vc->vc_cols && count)
        {
            if (attrib != (scr_readw(p).attr))
            {
                if (p > q)
                {
                    vc->vc_sw->con_putcs(vc, q, (size_t)((void *)p - (void *)q), startx, yy);
                }
                startx = xx;
                q = p;
                attrib = scr_readw(p).attr;
            }
            p++;
            xx++;
            count--;
        }
        if (p > q)
        {
            vc->vc_sw->con_putcs(vc, q, (size_t)((void *)p - (void *)q), startx, yy);
        }
        if (!count)
        {
            break;
        }
        xx = 0;
        yy++;
    }
}

static void insert_char(vc_data_t *vc, unsigned int nr)
{
    vt_cell_t *p = vc->vc_pos;

    // vc_uniscr_insert(vc, nr);
    scr_memmovew(p + nr, p, vc->vc_cols - vc->vc_x - nr);
    vt_cell_t blank_cell = {.bg = vc->vc_bg, .cp = ' '};
    scr_memsetw(p, blank_cell, nr);
    vc_clear_need_wrap(vc);
    if (con_should_update(vc))
    {
        vc_do_update_region(vc, p, vc->vc_cols - vc->vc_x);
    }
}

INTERNAL vt_cell_t softcursor_original = {.cp = (~0)};

INTERNAL void vc_add_softcursor(vc_data_t *vc)
{
    vt_cell_t i = *vc->vc_pos;
    uint32_t type = 0;

    if (!(type & CUR_SW))
    {
        return;
    }
    if (softcursor_original.cp != (~0))
    {
        return;
    }
    softcursor_original = i;
    // i |= CUR_SET(type);
    // i ^= CUR_CHANGE(type);
    if ((type & CUR_ALWAYS_BG) && (softcursor_original.bg & CUR_BG) == (i.bg & CUR_BG))
    {
        i.bg ^= CUR_BG;
    }
    if ((type & CUR_INVERT_FG_BG) && (i.fg & CUR_FG) == (i.bg & CUR_BG))
    {
        i.fg ^= CUR_FG;
    }
    *vc->vc_pos = i;
    if (con_should_update(vc))
    {
        vc_putc(vc, &i, vc->vc_y, vc->vc_x);
    }
}

INTERNAL void vc_hide_softcursor(vc_data_t *vc)
{
    if (softcursor_original.cp != (~0))
    {
        *vc->vc_pos = softcursor_original;
        if (con_should_update(vc))
        {
            vc_putc(vc, &softcursor_original, vc->vc_y, vc->vc_x);
        }
        softcursor_original.cp = (~0);
    }
}

INTERNAL void vc_hide_cursor(vc_data_t *vc)
{
    // if (vc_is_sel(vc))
    // {
    //     clear_selection();
    // }

    if (vc->vc_sw->con_cursor)
    {
        vc->vc_sw->con_cursor(vc, false);
    }
    vc_hide_softcursor(vc);
}

INTERNAL void vc_set_cursor(vc_data_t *vc)
{
    if (!vc_is_fg(vc))
    {
        return;
    }
    if (FLAG_IS_SET(vc->vc_modes, VC_MODE_CUR_VISIBLE))
    {
        // if (vc_is_sel(vc))
        // {
        //     clear_selection();
        // }
        vc_add_softcursor(vc);
        // if (CUR_SIZE(vc->cursor_type) != CUR_NONE)
        {
            vc->vc_sw->con_cursor(vc, true);
        }
    }
    else
    {
        vc_hide_cursor(vc);
    }
}

INTERNAL void vc_set_origin(vc_data_t *vc)
{
    if (!con_is_visible(vc) || !vc->vc_sw->con_set_origin || vc->vc_sw->con_set_origin(vc))
    {
        vc->vc_origin = vc->vc_screenbuf;
    }
    vc->vc_visible_origin = vc->vc_origin;
    vc->vc_scr_end = vc->vc_origin + vc->vc_screenbuf_cells;
    vc->vc_pos = vc_cell(vc, vc->vc_origin);
    // vc->vc_pos = (vc->vc_origin + vc->vc_y + sizeof(vt_cell_t) * vc->vc_x);
}

INTERNAL void vc_save_screen(vc_data_t *vc)
{
    if (vc->vc_sw->con_save_screen)
    {
        vc->vc_sw->con_save_screen(vc);
    }
}

static void con_scroll(vc_data_t *vc, uint32_t top, uint32_t bottom, con_scroll_t dir, uint32_t nr)
{
    ENTER_FUNC("%p, 0x%x, 0x%x, %u, %u", vc, top, bottom, dir, nr);
    uint32_t rows = bottom - top;
    vt_cell_t *clear;
    vt_cell_t *dst;
    vt_cell_t *src;

    if (top + nr >= bottom)
    {
        nr = rows - 1;
    }
    if (bottom > vc->vc_rows || top >= bottom || nr < 1)
    {
        return;
    }

    // vc_uniscr_scroll(vc, top, bottom, dir, nr);
    if (con_is_visible(vc) && vc->vc_sw->con_scroll(vc, top, bottom, dir, nr))
    {
        return;
    }

    src = clear = &vc->vc_origin[top * vc->vc_cols];
    dst = &vc->vc_origin[(top + nr) * vc->vc_cols];

    if (dir == SM_UP)
    {
        clear = &src[(rows - nr) * vc->vc_cols];
        swap(src, dst);
    }
    scr_memmovew(dst, src, rows - nr);
    vt_cell_t blank_cell = {.bg = vc->vc_bg, .cp = ' '};
    scr_memsetw(clear, blank_cell, nr);
}

static void lf(struct vc_data *vc)
{
    // ENTER_FUNC("%p", vc);
    // trace_debug(MODULE, "pos = %p, vc_y = %u", vc->vc_pos, vc->vc_y);
    if (vc->vc_y + 1 == vc->vc_bottom)
    {
        con_scroll(vc, vc->vc_top, vc->vc_bottom, SM_UP, 1);
    }
    else if (vc->vc_y < vc->vc_rows - 1)
    {
        vc->vc_y++;
        vc->vc_pos += 1;
    }
    // trace_debug(MODULE, "pos = %p, vc_y = %u", vc->vc_pos, vc->vc_y);
    vc_clear_need_wrap(vc);
    // notify_write(vc, '\n');
}

static void ri(struct vc_data *vc)
{
    if (vc->vc_y == vc->vc_top)
    {
        con_scroll(vc, vc->vc_top, vc->vc_bottom, SM_DOWN, 1);
    }
    else if (vc->vc_y > 0)
    {
        vc->vc_y--;
        vc->vc_pos -= 1;
    }
    vc_clear_need_wrap(vc);
}

static inline void cr(struct vc_data *vc)
{
    // ENTER_FUNC("%p", vc);
    // trace_debug(MODULE, "pos = %p, vc_x = %u", vc->vc_pos, vc->vc_x);
    vc->vc_pos -= vc->vc_x;
    vc_clear_need_wrap(vc);
    vc->vc_x = 0;
    // trace_debug(MODULE, "pos = %p, vc_x = %u", vc->vc_pos, vc->vc_x);
    // notify_write(vc, '\r');
}

static inline void bs(struct vc_data *vc)
{
    if (vc->vc_x)
    {
        vc->vc_pos -= 1;
        vc->vc_x--;
        vc_clear_need_wrap(vc);
        // notify_write(vc, '\b');
    }
}

static inline void del(struct vc_data *vc)
{
    /* ignored */
}

void vc_update_region(vc_data_t *vc, vt_cell_t *start, size_t count)
{
    WARN_LOCK_UNLOCKED();

    if (con_should_update(vc))
    {
        vc_hide_cursor(vc);
        vc_do_update_region(vc, start, count);
        vc_set_cursor(vc);
    }
}

void vc_clear_buffer_attributes(vc_data_t *vc)
{
    vt_cell_t *p = (vt_cell_t *)vc->vc_origin;
    int count = vc->vc_screenbuf_cells;
    // int mask = vc->vc_hi_font_mask | 0xFF;

    for (; count > 0; count--, p++)
    {
        p->attr = 0;
        p->bg = p->fg = 0;
        p->cp = ' ';
        // scr_writew((scr_readw(p) & mask) | (vc->vc_video_erase_char & ~mask), p);
    }
}

void vc_redraw_screen(vc_data_t *vc, int is_switch)
{
    ENTER_FUNC("%p, %s", vc, is_switch BOOL_TO_STRING);
    int redraw = 0;

    WARN_LOCK_UNLOCKED();
    trace_debug(MODULE, "here");

    if (!vc)
    {
        /* strange ... */
        /* printk("vc_redraw_screen: tty %d not allocated ??\n", new_console+1); */
        return;
    }
    trace_debug(MODULE, "here 1");

    if (is_switch)
    {
        vc_data_t *old_vc = consoles[fg_console].data;
        if (old_vc == vc)
        {
            return;
        }
        if (!con_is_visible(vc))
        {
            redraw = 1;
        }
        *vc->vc_display_fg = vc;
        fg_console = vc->vc_index;
        vc_hide_cursor(old_vc);
        if (!con_is_visible(old_vc))
        {
            vc_save_screen(old_vc);
            vc_set_origin(old_vc);
        }
    }
    else
    {
        vc_hide_cursor(vc);
        redraw = 1;
    }

    trace_debug(MODULE, "here 2");
    if (redraw)
    {
        bool update;
        bool old_was_color = vc_can_color(vc);

        trace_debug(MODULE, "here 3");
        vc_set_origin(vc);
        trace_debug(MODULE, "here 4");
        update = vc->vc_sw->con_switch(vc);
        // set_palette(vc);

        if (old_was_color != vc_can_color(vc))
        {
            // update_attr(vc);
            vc_clear_buffer_attributes(vc);
        }

        if (update && vc_use_text(vc))
        {
            vc_do_update_region(vc, vc->vc_origin, vc->vc_screenbuf_cells);
        }
    }
    vc_set_cursor(vc);
    if (is_switch)
    {
        // vt_set_leds_compute_shiftstate();
        // notify_update(vc);
        FUNC_NOT_IMPLEMENTED();
    }
}

INTERNAL void vc_gotoxy(vc_data_t *vc, uint32_t new_x, uint32_t new_y)
{
    if (new_x < 0)
    {
        vc->vc_x = 0;
    }
    else
    {
        if (new_x >= vc->vc_cols)
        {
            vc->vc_x = vc->vc_cols - 1;
        }
        else
        {
            vc->vc_x = new_x;
        }
    }

    uint32_t min_y;
    uint32_t max_y;
    if (FLAG_IS_SET(vc->vc_modes, VC_MODE_ORIGIN_MODE))
    {
        min_y = vc->vc_top;
        max_y = vc->vc_bottom;
    }
    else
    {
        min_y = 0;
        max_y = vc->vc_rows;
    }
    if (new_y < min_y)
    {
        vc->vc_y = min_y;
    }
    else if (new_y >= max_y)
    {
        vc->vc_y = max_y - 1;
    }
    else
    {
        vc->vc_y = new_y;
    }
    // vc->vc_pos = (((vt_cell_t *)vc->vc_origin) + vc->vc_y * vc->vc_cols + vc->vc_x);
    vc->vc_pos = vc_cell(vc, vc->vc_origin);
    FLAG_UNSET(vc->vc_modes, VC_MODE_NEED_WRAP);
}

INTERNAL void vc_save_cur(vc_data_t *vc)
{
    vc->vc_saved_x = vc->vc_x;
    vc->vc_saved_y = vc->vc_y;
    vc->vc_saved_fg = vc->vc_fg;
    vc->vc_saved_bg = vc->vc_bg;
    vc->vc_saved_attr = vc->vc_attr;
}

INTERNAL void vc_restore_cur(vc_data_t *vc)
{
    vc->vc_x = vc->vc_saved_x;
    vc->vc_y = vc->vc_saved_y;
    vc->vc_fg = vc->vc_saved_fg;
    vc->vc_bg = vc->vc_saved_bg;
    vc->vc_attr = vc->vc_saved_attr;
    vc_gotoxy(vc, vc->vc_x, vc->vc_y);

    FLAG_UNSET(vc->vc_modes, VC_MODE_NEED_WRAP);
}

static void vc_reset_terminal(vc_data_t *vc, int do_clear)
{
    unsigned int i;

    vc->vc_top = 0;
    vc->vc_bottom = vc->vc_rows;
    vc->vc_state = ESnormal;
    vc_set_use_utf8(vc);
    vc->vc_utf_count = 0;

    BITMAP_ZERO(vc->vc_tab_stop, VC_TABSTOPS_COUNT);
    for (i = 0; i < VC_TABSTOPS_COUNT; i += 8)
    {
        SET_BIT(i, vc->vc_tab_stop);
    }

    vc_gotoxy(vc, 0, 0);
    vc_save_cur(vc);
}

static color_t default_color = 0x00FFFFFF;           /* white */
static color_t default_italic_color = 0x0000AA00;    // green (ASCII)
static color_t default_underline_color = 0x0000AAAA; // cyan (ASCII)

INTERNAL void vc_init(vc_data_t *vc, int do_clear)
{
    vc_set_origin(vc);
    vc->vc_pos = (vt_cell_t *)vc->vc_origin;
    vc_set_use_text(vc);
    int j, k;
    for (j = k = 0; j < 16; j++)
    {
        vc->vc_palette[k++] = default_color_table[j];
    }
    vc->vc_def_fg = vc->vc_fg = default_color;
    // init_waitqueue_head(&vc->paste_wait);
    vc_reset_terminal(vc, do_clear);
}

#define VC_MAXCOL (32767)
#define VC_MAXROW (32767)

status_t vc_allocate(uint32_t currcons)
{
    ENTER_FUNC("%u", currcons)
    vc_data_t *vc;
    status_t ret;

    WARN_LOCK_UNLOCKED();

    if (currcons >= MAX_CONSOLES)
    {
        KERRNO_RETURN(ENXIO, "");
    }

    if (consoles[currcons].data)
    {
        return KERRNO_SUCCESSES;
    }

    vc = kzmalloc(sizeof(vc_data_t));
    if (!vc)
    {
        KERRNO_RETURN(KERRNO_OOM, "");
    }

    consoles[currcons].data = vc;

    ret = vc_data_init(vc, currcons, true);
    if (ret)
    {
        KERRNO_NO_RETURN(ret, "");
        goto err_free;
    }

    if (vc->vc_cols > VC_MAXCOL || vc->vc_rows > VC_MAXROW || !vc->vc_screenbuf_cells)
    {
        ret = KERRNO_BAD_VALUE;
        KERRNO_NO_RETURN(KERRNO_BAD_VALUE, "Size is invalid");
        trace_debug(MODULE, "vc->size = { %ux%u }, vc->vc_screenbuf_cells = %u", vc->vc_cols, vc->vc_rows, vc->vc_screenbuf_cells);
        goto err_free;
    }

    vc->vc_screenbuf = kzmalloc(vc->vc_screenbuf_cells * sizeof(vt_cell_t));
    if (!vc->vc_screenbuf)
    {
        KERRNO_NO_RETURN(KERRNO_OOM, "");
        ret = KERRNO_OOM;
        goto err_free;
    }

    vc_init(vc, 1);

    return KERRNO_SUCCESSES;
err_free:
    vt_data_deinit(vc);
    kfree(vc);
    consoles[currcons].data = NULL;
    return ret;
}

INTERNAL ALWAYS_INLINE status_t vc_resize_screen(vc_data_t *vc, int width, int height, bool from_user)
{
    ENTER_FUNC("%p, %u, %u, %s", vc, width, height, from_user BOOL_TO_STRING);
    /* Resizes the resolution of the display adapater */
    int err = 0;

    if (vc->vc_sw->con_resize)
    {
        err = vc->vc_sw->con_resize(vc, width, height, from_user);
    }

    return err;
}

INTERNAL status_t vc_do_resize(tty_struct_t *tty, vc_data_t *vc, uint32_t cols, uint32_t lines, bool from_user)
{
    ENTER_FUNC("%p, %p, %u, %u, %s", tty, vc, cols, lines, from_user BOOL_TO_STRING);
    status_t err = 0;

    WARN_LOCK_UNLOCKED();

    if (cols > VC_MAXCOL || lines > VC_MAXROW)
    {
        KERRNO_RETURN(EINVAL, "");
    }

    uint32_t new_cols = (cols ? cols : vc->vc_cols);
    uint32_t new_rows = (lines ? lines : vc->vc_rows);
    size_t new_row_size = new_cols << 4;
    size_t new_screen_size = new_rows * new_cols;

    if (new_cols == vc->vc_cols && new_rows == vc->vc_rows)
    {
        return vc_resize_screen(vc, new_cols, new_rows, from_user);
    }

    if (!new_screen_size)
    {
        KERRNO_RETURN(EINVAL, "");
    }
    vt_cell_t *newscreen = kzmalloc(new_screen_size * sizeof(vt_cell_t));
    if (!newscreen)
    {
        KERRNO_RETURN(ENOMEM, "");
    }

    // if (vc->vc_uni_lines)
    // {
    //     new_uniscr = vc_uniscr_alloc(new_cols, new_rows);
    //     if (!new_uniscr)
    //     {
    //         kfree(newscreen);
    //         return -ENOMEM;
    //     }
    // }

    // if (vc_is_sel(vc))
    // {
    //     clear_selection();
    // }

    uint32_t old_rows = vc->vc_rows;
    uint32_t old_cols = vc->vc_cols;
    size_t old_row_size = (old_cols << 4);

    err = vc_resize_screen(vc, new_cols, new_rows, from_user);
    if (err)
    {
        kfree(newscreen);
        // vc_uniscr_free(new_uniscr);
        return err;
    }

    vc->vc_rows = new_rows;
    vc->vc_cols = new_cols;
    // vc->vc_size_row = new_row_size;
    vc->vc_screenbuf_cells = new_screen_size;

    uint32_64_t rlth = min(old_row_size, new_row_size);
    uint32_64_t rrem = new_row_size - rlth;
    vt_cell_t *old_origin = vc->vc_origin;
    vt_cell_t *new_origin = newscreen;
    vt_cell_t *new_scr_end = &new_origin[new_screen_size];

    uint32_t first_copied_row = 0;
    if (vc->vc_y > new_rows)
    {
        if (old_rows - vc->vc_y < new_rows)
        {
            first_copied_row = (old_rows - new_rows);
        }
        else
        {
            first_copied_row = (vc->vc_y - new_rows);
        }
        old_origin += first_copied_row;
    }
    else
    {
        first_copied_row = 0;
    }
    vt_cell_t *end = &old_origin[min(old_rows, new_rows)];

    // vc_uniscr_copy_area(new_uniscr, new_cols, new_rows, vc->vc_uni_lines, rlth / 2, first_copied_row, min(old_rows, new_rows));
    // vc_uniscr_set(vc, new_uniscr);

    // vc_update_attr(vc);

    trace_debug(MODULE, "newscreen = %p", newscreen);
    trace_debug(MODULE, "new_origin = %p, old_origin = %p", new_origin, old_origin);
    trace_debug(MODULE, "end = %p", end);
    trace_debug(MODULE, "new_row_size = %u, old_row_size = %u", new_row_size, old_row_size);
    trace_debug(MODULE, "new_cols = %u, old_cols = %u", new_cols, old_cols);
    trace_debug(MODULE, "new_rows = %u, old_rows = %u", new_rows, old_rows);
    trace_debug(MODULE, "rlth = %u, rrem = %u", rlth, rrem);
    while (old_origin < end)
    {
        memcpy((vt_cell_t *)new_origin, (vt_cell_t *)old_origin, sizeof(vt_cell_t));
        // if (rrem)
        // {
        //     memset((void *)(new_origin + rlth * sizeof(vt_cell_t)), 0, rrem * sizeof(vt_cell_t));
        // }
        old_origin += 1;
        new_origin += 1;
    }
    trace_debug(MODULE, "new_origin = %p, new_scr_end = %p, old_origin = %p, new_scr_end - new_origin = %p", new_origin, new_scr_end, old_origin, new_scr_end - new_origin);
    if (new_scr_end > new_origin)
    {
        memset((void *)new_origin, 0, new_scr_end - new_origin);
    }
    vt_cell_t *oldscreen = vc->vc_screenbuf;
    vc->vc_screenbuf = newscreen;
    vc->vc_screenbuf_cells = new_screen_size;
    vc_set_origin(vc);
    trace_debug(MODULE, "oldscreen = %p", oldscreen);
    kfree(oldscreen);

    /* do part of a vc_reset_terminal() */
    vc->vc_top = 0;
    vc->vc_bottom = vc->vc_rows;
    vc_gotoxy(vc, vc->vc_x, vc->vc_y);
    vc_save_cur(vc);

    if (tty)
    {
        /* Rewrite the requested winsize data with the actual
           resulting sizes */
        winsize_t ws;
        memset(&ws, 0, sizeof(ws));
        ws.ws_row = vc->vc_rows;
        ws.ws_col = vc->vc_cols;
        // ws.ws_ypixel = vc->vc_size_row;
        tty_do_resize(tty, &ws);
    }

    if (con_is_visible(vc))
    {
        update_screen(vc);
    }
    // vt_event_post(VT_EVENT_RESIZE, vc->vc_num, vc->vc_num);
    // notify_update(vc);
    return err;
}

status_t vc_resize(vc_data_t *vc, uint32_t cols, uint32_t rows, bool from_user)
{
    ENTER_FUNC("%p, %u, %u, %s", vc, cols, rows, from_user BOOL_TO_STRING);
    tty_struct_t *tty = vc->vc_tty;
    // tty_dev_t *tty_dev = tty->tty_dev;
    // LOCK();
    status_t ret = vc_do_resize(tty, vc, cols, rows, from_user);
    // UNLOCK();
    return ret;
}

static status_t vt_resize(tty_struct_t *tty, winsize_t *ws)
{
    vc_data_t *vc = tty->tty_dev->driver_data;

    // guard(console_lock)();
    return vc_do_resize(tty, vc, ws->ws_col, ws->ws_row, false);
}

int vc_cons_allocated(unsigned int i)
{
    return i < MAX_CONSOLES && consoles[i].data;
}

status_t vt_console_added()
{
    return KERRNO_SUCCESSES;
}

status_t vt_console_removed()
{
    return KERRNO_SUCCESSES;
}

typedef struct vc_draw_region
{
    vt_cell_t *from;
    vt_cell_t *to;
    int x;
} vc_draw_region_t;

static void vt_con_flush(vc_data_t *vc, vc_draw_region_t *draw)
{
    ENTER_FUNC("%p, %p", vc, draw);
    // trace_debug(MODULE, "draw->x = %u", draw->x);
    // trace_debug(MODULE, "draw->to = %p", draw->to);
    // trace_debug(MODULE, "draw->from = %p", draw->from);
    if (draw->x < 0)
    {
        return;
    }

    vc->vc_sw->con_putcs(vc, draw->from, draw->to - draw->from, draw->x, vc->vc_y);
    draw->x = -1;
}

INTERNAL status_t vc_con_write_normal(vc_data_t *vc, code_point_t tc, code_point_t c, vc_draw_region_t *draw)
{
    // ENTER_FUNC("%p, 0x%x, 0x%x, %p", vc, tc, c, draw);

    // int next_c;
    uint32_t vc_attr = vc->vc_attr;
    uint8_t width = 1;
    // bool inverse = false;

    // if (vc->vc_utf && !vc->vc_disp_ctrl)
    // {
    //     width = vc_process_ucs(vc, &c, &tc);
    //     if (!width)
    //     {
    //         goto out;
    //     }
    // }

    // tc = vc_get_glyph(vc, tc);
    // if (tc == -1)
    // {
    //     return -1; /* nothing to display */
    // }
    // if (tc < 0)
    // {
    //     inverse = true;
    //     tc = conv_uni_to_pc(vc, '?');
    //     if (tc < 0)
    //     {
    //         tc = '?';
    //     }

    //     vc_attr = vc_invert_attr(vc);
    //     con_flush(vc, draw);
    // }

    // next_c = c;
    while (1)
    {
        if (vc_need_wrap(vc) || vc_decim_mode(vc))
        {
            vt_con_flush(vc, draw);
        }
        if (vc_need_wrap(vc))
        {
            cr(vc);
            lf(vc);
        }
        if (vc_decim_mode(vc))
        {
            insert_char(vc, 1);
        }
        // vc_uniscr_putc(vc, next_c);

        vt_cell_t cell = {.attr = vc_attr, .bg = vc->vc_bg, .fg = vc->vc_fg, .cp = tc};

        scr_writew(cell, vc->vc_pos);

        if (con_should_update(vc) && draw->x < 0)
        {
            draw->x = vc->vc_x;
            draw->from = vc->vc_pos;
        }
        if (vc->vc_x == vc->vc_cols - 1)
        {
            vc_assign_need_wrap(vc, vc_decawm_mode(vc));
            draw->to = vc->vc_pos + 1;
        }
        else
        {
            vc->vc_x++;
            draw->to = (vc->vc_pos += 1);
        }

        if (!--width)
        {
            break;
        }
    }

    // trace_debug(MODULE, "draw->x = %u", draw->x);
    // trace_debug(MODULE, "draw->to = %p", draw->to);
    // trace_debug(MODULE, "draw->from = %p", draw->from);

    // vc->vc_pos->cp = tc;
    // vc->vc_pos->bg = vc->vc_bg;
    // vc->vc_pos->fg = vc->vc_fg;
    // vc->vc_pos->attr = vc->vc_attr;
    // vc->vc_pos++;

    return KERRNO_SUCCESSES;
}

static bool ansi_control_string(enum vc_ctl_state state)
{
    return state >= ESANSI_first && state <= ESANSI_last;
}

static bool handle_ascii(tty_struct_t *tty, vc_data_t *vc, uint8_t c)
{
    ENTER_FUNC("%p, %p, 0x%x", tty, vc, c);
    switch (c)
    {
        case ASCII_NULL :
            return true;
        case ASCII_BELL :
            if (ansi_control_string(vc->vc_state))
            {
                vc->vc_state = ESnormal;
            }
            // else if (vc->vc_bell_duration)
            // {
            // kd_mksound(vc->vc_bell_pitch, vc->vc_bell_duration);
            // }
            return true;
        case ASCII_BACKSPACE :
            return true;
        case ASCII_HTAB :
            {
                void *tmp_pos = vc->vc_pos;
                tmp_pos -= vc->vc_x;
                vc->vc_pos = tmp_pos;

                vc->vc_x = find_next_bit(vc->vc_tab_stop, min(vc->vc_cols - 1, VC_TABSTOPS_COUNT), vc->vc_x + 1);
                if (vc->vc_x >= VC_TABSTOPS_COUNT)
                {
                    vc->vc_x = vc->vc_cols - 1;
                }

                tmp_pos = vc->vc_pos;
                tmp_pos += (vc->vc_x);
                vc->vc_pos = tmp_pos;
            }
            return true;
        case ASCII_LINEFEED :
        case ASCII_VTAB :
        case ASCII_FORMFEED :
            lf(vc);
            // if (!is_kbd(vc, lnm))
            // {
            // return true;
            // }
            cr(vc);
            return true;
        case ASCII_CAR_RET :
            cr(vc);
            return true;
        case ASCII_SHIFTOUT :
            // vc->state.charset = 1;
            // vc->vc_translate = set_translate(vc->state.Gx_charset[1], vc);
            // vc->vc_disp_ctrl = 1;
            return true;
        case ASCII_SHIFTIN :
            // vc->state.charset = 0;
            // vc->vc_translate = set_translate(vc->state.Gx_charset[0], vc);
            // vc->vc_disp_ctrl = 0;
            return true;
        case ASCII_CANCEL :
        case ASCII_SUBSTITUTE :
            vc->vc_state = ESnormal;
            return true;
        case ASCII_ESCAPE :
            vc->vc_state = ESesc;
            return true;
        case ASCII_DEL :
            del(vc);
            return true;
        case ASCII_EXT_CSI :
            vc->vc_state = ESsquare;
            return true;
    }

    return false;
}

static void do_con_trol(tty_struct_t *tty, vc_data_t *vc, uint8_t c)
{
    if (ansi_control_string(vc->vc_state) && c >= ASCII_IGNORE_FIRST && c <= ASCII_IGNORE_LAST)
    {
        return;
    }

    if (handle_ascii(tty, vc, c))
    {
        return;
    }

    FUNC_NOT_IMPLEMENTED();

    /*
    switch (vc->vc_state)
    {
        case ESesc :
            handle_esc(tty, vc, c);
            return;
        case ESnonstd :
            switch (c)
            {
                case 'P' :
                    vc_reset_params(vc);
                    vc->vc_state = ESpalette;
                    return;
                case 'R' :
                    reset_palette(vc);
                    break;
                case '0' ... '9' :
                    vc->vc_state = ESosc;
                    return;
            }
            vc->vc_state = ESnormal;
            return;
        case ESpalette :
            if (isxdigit(c))
            {
                vc->vc_par[vc->vc_npar++] = hex_to_bin(c);
                if (vc->vc_npar == 7)
                {
                    int i = vc->vc_par[0] * 3, j = 1;
                    vc->vc_palette[i] = 16 * vc->vc_par[j++];
                    vc->vc_palette[i++] += vc->vc_par[j++];
                    vc->vc_palette[i] = 16 * vc->vc_par[j++];
                    vc->vc_palette[i++] += vc->vc_par[j++];
                    vc->vc_palette[i] = 16 * vc->vc_par[j++];
                    vc->vc_palette[i] += vc->vc_par[j];
                    set_palette(vc);
                    vc->vc_state = ESnormal;
                }
            }
            else
            {
                vc->vc_state = ESnormal;
            }
            return;
        case ESsquare :
            vc_reset_params(vc);

            vc->vc_state = ESgetpars;
            switch (c)
            {
                case '[' :
                    vc->vc_state = ESfunckey;
                    return;
                case '?' :
                    vc->vc_priv = EPdec;
                    return;
                case '>' :
                    vc->vc_priv = EPgt;
                    return;
                case '=' :
                    vc->vc_priv = EPeq;
                    return;
                case '<' :
                    vc->vc_priv = EPlt;
                    return;
            }
            vc->vc_priv = EPecma;
            fallthrough;
        case ESgetpars :
            switch (c)
            {
                case ':' :
                    if (vc->vc_par[vc->vc_npar] == CSI_m_FG_COLOR ||
                        vc->vc_par[vc->vc_npar] == CSI_m_BG_COLOR ||
                        vc->vc_par[vc->vc_npar] == CSI_m_UNDERLINE_COLOR)
                    {
                        vc->vc_state = ESgetsubpars;
                    }
                    else
                    {
                        break;
                    }
                    fallthrough;
                case ';' :
                    if (vc->vc_npar < NPAR - 1)
                    {
                        vc->vc_npar++;
                        return;
                    }
                    break;
                case '0' ... '9' :
                    vc->vc_par[vc->vc_npar] *= 10;
                    vc->vc_par[vc->vc_npar] += c - '0';
                    return;
            }
            if (c >= ASCII_CSI_IGNORE_FIRST && c <= ASCII_CSI_IGNORE_LAST)
            {
                vc->vc_state = EScsiignore;
                return;
            }

            vc->vc_state = ESnormal;

            switch (vc->vc_priv)
            {
                case EPdec :
                    csi_DEC(tty, vc, c);
                    return;
                case EPecma :
                    csi_ECMA(tty, vc, c);
                    return;
                default :
                    return;
            }
        case ESgetsubpars :
            switch (c)
            {
                case ':' :
                case ';' :
                    if (vc->vc_npar < NPAR - 1)
                    {
                        vc->vc_npar++;
                        return;
                    }
                    break;
                case '0' ... '9' :
                    vc->vc_par[vc->vc_npar] *= 10;
                    vc->vc_par[vc->vc_npar] += c - '0';
                    return;
            }
            if (c >= ASCII_CSI_IGNORE_FIRST && c <= ASCII_CSI_IGNORE_LAST)
            {
                vc->vc_state = EScsiignore;
                return;
            }

            vc->vc_state = ESnormal;

            switch (vc->vc_priv)
            {
                case EPdec :
                    csi_DEC(tty, vc, c);
                    return;
                case EPecma :
                    csi_ECMA(tty, vc, c);
                    return;
                default :
                    return;
            }
        case EScsiignore :
            if (c >= ASCII_CSI_IGNORE_FIRST && c <= ASCII_CSI_IGNORE_LAST)
            {
                return;
            }
            vc->vc_state = ESnormal;
            return;
        case ESpercent :
            vc->vc_state = ESnormal;
            switch (c)
            {
                case '@' :
                    vc->vc_utf = 0;
                    return;
                case 'G' :
                case '8' :
                    vc->vc_utf = 1;
                    return;
            }
            return;
        case ESfunckey :
            vc->vc_state = ESnormal;
            return;
        case EShash :
            vc->vc_state = ESnormal;
            if (c == '8')
            {
                vc->vc_video_erase_char =
                    (vc->vc_video_erase_char & 0xFF00) | 'E';
                csi_J(vc, CSI_J_VISIBLE);
                vc->vc_video_erase_char =
                    (vc->vc_video_erase_char & 0xFF00) | ' ';
                do_update_region(vc, vc->vc_origin, vc->vc_screenbuf_size / 2);
            }
            return;
        case ESsetG0 :
            vc_setGx(vc, 0, c);
            vc->vc_state = ESnormal;
            return;
        case ESsetG1 :
            vc_setGx(vc, 1, c);
            vc->vc_state = ESnormal;
            return;
        case ESapc :
            return;
        case ESosc :
            return;
        case ESpm :
            return;
        case ESdcs :
            return;
        default :
            vc->vc_state = ESnormal;
    }
    */
}

static inline code_point_t vc_translate_ascii(const vc_data_t *vc, code_point_t c)
{
    return c;
}

static inline code_point_t vc_sanitize_unicode(const code_point_t c)
{
    if (c >= 0xD800 && c <= 0xDFFF)
    {
        return 0xFFFD;
    }

    return c;
}

static code_point_t vc_translate_unicode(vc_data_t *vc, code_point_t c, bool *rescan)
{
    static const uint32_t utf8_length_changes[] = {0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

    if ((c & 0xC0) == 0x80)
    {
        if (!vc->vc_utf_count)
        {
            goto bad_sequence;
        }

        vc->vc_utf_char = (vc->vc_utf_char << 6) | (c & 0x3F);
        vc->vc_npar++;
        if (--vc->vc_utf_count)
        {
            goto need_more_bytes;
        }

        c = vc->vc_utf_char;
        if (c <= utf8_length_changes[vc->vc_npar - 1] || c > utf8_length_changes[vc->vc_npar])
        {
            goto bad_sequence;
        }

        return vc_sanitize_unicode(c);
    }

    if (vc->vc_utf_count)
    {
        *rescan = true;
        vc->vc_utf_count = 0;
        goto bad_sequence;
    }

    if (c <= 0x7F)
    {
        return c;
    }

    vc->vc_npar = 0;
    if ((c & 0xE0) == 0xC0)
    {
        vc->vc_utf_count = 1;
        vc->vc_utf_char = (c & 0x1F);
    }
    else if ((c & 0xF0) == 0xE0)
    {
        vc->vc_utf_count = 2;
        vc->vc_utf_char = (c & 0x0F);
    }
    else if ((c & 0xF8) == 0xF0)
    {
        vc->vc_utf_count = 3;
        vc->vc_utf_char = (c & 0x07);
    }
    else
    {
        goto bad_sequence;
    }

need_more_bytes:
    return -1;

bad_sequence:
    return 0xFFFD;
}

static code_point_t vc_translate(vc_data_t *vc, code_point_t *c, bool *rescan)
{
    if (vc->vc_state != ESnormal)
    {
        return *c;
    }

    if (vc_use_utf8(vc))
    {
        return *c = vc_translate_unicode(vc, *c, rescan);
    }

    return vc_translate_ascii(vc, *c);
}

static bool vc_is_control(vc_data_t *vc, code_point_t tc, code_point_t c)
{
    static const uint32_t CTRL_ACTION = BIT(ASCII_NULL) |
        GENMASK(ASCII_SHIFTIN, ASCII_BELL) | BIT(ASCII_CANCEL) |
        BIT(ASCII_SUBSTITUTE) | BIT(ASCII_ESCAPE);
    /* Cannot be overridden by disp_ctrl */
    static const uint32_t CTRL_ALWAYS = BIT(ASCII_NULL) | BIT(ASCII_BACKSPACE) |
        BIT(ASCII_LINEFEED) | BIT(ASCII_SHIFTIN) | BIT(ASCII_SHIFTOUT) |
        BIT(ASCII_CAR_RET) | BIT(ASCII_FORMFEED) | BIT(ASCII_ESCAPE);
    if (vc->vc_state != ESnormal)
    {
        return true;
    }

    if (!tc)
    {
        return true;
    }

    if (c < BITS_PER_TYPE(CTRL_ALWAYS))
    {
        // if (vc->vc_disp_ctrl)
        // {
        //     return CTRL_ALWAYS & BIT(c);
        // }
        // else
        {
            return vc_use_utf8(vc) || (CTRL_ACTION & BIT(c));
        }
    }

    if (c == ASCII_DEL)
    {
        return true;
    }

    if (c == ASCII_EXT_CSI)
    {
        return true;
    }

    return false;
}

status_t vt_do_con_write(tty_struct_t *tty, const uint8_t *buf, size_t count, size_t *size_out)
{
    ENTER_FUNC("%p, %p, %u, %p", tty, buf, count, size_out);
    vc_data_t *vc = tty->tty_dev->driver_data;
    trace_debug(MODULE, "vc = %p", vc);
    vc_draw_region_t draw = {
        .x = -1,
    };
    int n = 0;
    unsigned int currcons;

    LOCK();
    currcons = vc->vc_index;
    if (!vc_cons_allocated(currcons))
    {
        UNLOCK();
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "con_write: console %d not allocated", currcons);
    }

    if (vc_is_fg(vc))
    {
        vc_hide_cursor(vc);
    }
    while (!tty->flow.stopped && count)
    {
        uint8_t orig = *buf;
        buf++;
        n++;
        count--;
rescan_last_byte:
        code_point_t c = orig;
        code_point_t tc = 0;
        bool rescan = false;

        tc = vc_translate(vc, &c, &rescan);
        if (tc == -1)
        {
            continue;
        }

        if (vc_is_control(vc, tc, c))
        {
            vt_con_flush(vc, &draw);
            do_con_trol(tty, vc, orig);
            continue;
        }

        if (vc_con_write_normal(vc, tc, c, &draw))
        {
            continue;
        }

        if (rescan)
        {
            goto rescan_last_byte;
        }
    }
    // draw.to = draw.from + n;
    vt_con_flush(vc, &draw);
    // notify_update(vc);

    // trace_debug(MODULE, "pos = %p, vc_x = %u", vc->vc_pos, vc->vc_x);
    *size_out = n;
    UNLOCK();
    return KERRNO_SUCCESSES;
}

status_t con_vc_install(tty_driver_t *driver, tty_struct_t *tty)
{
    ENTER_FUNC("%p, %p", driver, tty);
    unsigned int currcons = tty->index;
    vc_data_t *vc;
    int ret;

    LOCK();
    ret = vc_allocate(currcons);
    if (ret)
    {
        UNLOCK();
        return ret;
    }

    vc = consoles[currcons].data;

    if (vc->vc_tty)
    {
        KERRNO_RETURN(KERRNO_INTERRUPTED, "");
    }

    tty->tty_dev->driver_data = vc;
    vc->vc_tty = tty;

    if (!tty->winsize.ws_row && !tty->winsize.ws_col)
    {
        tty->winsize.ws_row = consoles[currcons].data->vc_rows;
        tty->winsize.ws_col = consoles[currcons].data->vc_cols;
    }
    if (vc_use_utf8(vc))
    {
        tty->termios.c_iflag |= IUTF8;
    }
    else
    {
        tty->termios.c_iflag &= ~IUTF8;
    }

    UNLOCK();

    return KERRNO_SUCCESSES;
}

status_t con_vc_open(tty_struct_t *tty, vfs_node_t *filp)
{
    /* everything done in install */
    return KERRNO_SUCCESSES;
}

status_t con_vc_close(tty_struct_t *tty, vfs_node_t *filp)
{
    /* Nothing to do - we defer to shutdown */
    return KERRNO_SUCCESSES;
}

status_t vc_con_write(tty_struct_t *tty, const uint8_t *buf, size_t count, ssize_t *size_out)
{
    ENTER_FUNC("%p, %p, %u, %p", tty, buf, count, size_out);
    size_t si;
    status_t ret = vt_do_con_write(tty, buf, count, &si);
    if (size_out)
    {
        *size_out = si;
    }
    return ret;
}

status_t con_put_char(tty_struct_t *tty, uint8_t ch)
{
    size_t si;
    return vt_do_con_write(tty, &ch, 1, &si);
}

tty_driver_t *console_driver;

#ifdef CONFIG_VT_CONSOLE

status_t vt_console_print(console_t *co, wchar_t *b, size_t count)
{
    //     struct vc_data *vc = vc_cons[fg_console].d;
    //     unsigned char c;
    //     static DEFINE_SPINLOCK(printing_lock);
    //     const ushort *start;
    //     ushort start_x, cnt;
    //     int kmsg_console;

    //     WARN_CONSOLE_UNLOCKED();

    //     /* this protects against concurrent oops only */
    //     if (!spin_trylock(&printing_lock))
    //     {
    //         return;
    //     }

    //     kmsg_console = vt_get_kmsg_redirect();
    //     if (kmsg_console && vc_cons_allocated(kmsg_console - 1))
    //     {
    //         vc = vc_cons[kmsg_console - 1].d;
    //     }

    //     if (!vc_cons_allocated(fg_console))
    //     {
    //         /* impossible */
    //         /* printk("vt_console_print: tty %d not allocated ??\n", currcons+1); */
    //         goto quit;
    //     }

    //     if (vc->vc_mode != KD_TEXT)
    //     {
    //         goto quit;
    //     }

    //     /* undraw cursor first */
    //     if (con_is_fg(vc))
    //     {
    //         hide_cursor(vc);
    //     }

    //     start = (ushort *)vc->vc_pos;
    //     start_x = vc->vc_x;
    //     cnt = 0;
    //     while (count--)
    //     {
    //         c = *b++;
    //         if (c == ASCII_LINEFEED || c == ASCII_CAR_RET ||
    //             c == ASCII_BACKSPACE || vc->vc_need_wrap)
    //         {
    //             if (cnt && con_is_visible(vc))
    //             {
    //                 vc->vc_sw->con_putcs(vc, start, cnt, vc->vc_y, start_x);
    //             }
    //             cnt = 0;
    //             if (c == ASCII_BACKSPACE)
    //             {
    //                 bs(vc);
    //                 start = (ushort *)vc->vc_pos;
    //                 start_x = vc->vc_x;
    //                 continue;
    //             }
    //             if (c != ASCII_CAR_RET)
    //             {
    //                 lf(vc);
    //             }
    //             cr(vc);
    //             start = (ushort *)vc->vc_pos;
    //             start_x = vc->vc_x;
    //             if (c == ASCII_LINEFEED || c == ASCII_CAR_RET)
    //             {
    //                 continue;
    //             }
    //         }
    //         vc_uniscr_putc(vc, c);
    //         scr_writew((vc->vc_attr << 8) + c, (unsigned short *)vc->vc_pos);
    //         notify_write(vc, c);
    //         cnt++;
    //         if (vc->vc_x == vc->vc_cols - 1)
    //         {
    //             vc->vc_need_wrap = 1;
    //         }
    //         else
    //         {
    //             vc->vc_pos += 2;
    //             vc->vc_x++;
    //         }
    //     }
    //     if (cnt && con_is_visible(vc))
    //     {
    //         vc->vc_sw->con_putcs(vc, start, cnt, vc->vc_y, start_x);
    //     }
    //     set_cursor(vc);
    //     notify_update(vc);

    // quit:
    //     spin_unlock(&printing_lock);
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESSES;
}

status_t vt_console_device(console_t *c, int *index, tty_driver_t **tty_out)
{
    *index = c->index ? c->index - 1 : fg_console;
    *tty_out = console_driver;
    return KERRNO_SUCCESSES;
}

status_t vt_console_setup(console_t *co, char *options)
{
    return co->index >= MAX_CONSOLES ? EINVAL : KERRNO_SUCCESSES;
}

console_t vt_console_driver = {
    .name = "tty",
    .setup = vt_console_setup,
    .write = vt_console_print,
    .device = vt_console_device,
    // .unblank	= unblank_screen,
    .flags = CON_PRINTBUFFER,
    .index = -1,
};
#endif

static const tty_ops_t con_ops = {
    .install = con_vc_install,
    .open = con_vc_open,
    .close = con_vc_close,
    .write = vc_con_write,
    // .write_room = con_write_room,
    .put_char = con_put_char,
    // 	.flush_chars = con_flush_chars,
    // 	.stop = con_stop,
    // 	.start = con_start,
    // 	.throttle = con_throttle,
    // 	.unthrottle = con_unthrottle,
    // 	.resize = vt_resize,
    // 	.shutdown = con_shutdown,
    // 	.cleanup = con_cleanup,
    // 	.ldisc_ok = con_ldisc_ok,
};

status_t vty_init()
{
    status_t ret = tty_alloc_driver(MAX_CONSOLES, TTY_DRIVER_REAL_RAW | TTY_DRIVER_RESET_TERMIOS, &console_driver);
    if (ret)
    {
        KERNEL_PANIC(MODULE, "Couldn't allocate console driver\n");
    }

    console_driver->name = "vtty";
    console_driver->major = tty_class.type;
    console_driver->device = 0;
    console_driver->name_base = 1;
    console_driver->minor_start = 1;
    console_driver->type = TTY_DRIVER_TYPE_CONSOLE;
    console_driver->init_termios = tty_std_termios;
    console_driver->init_termios.c_iflag |= IUTF8;
    console_driver->ops = &con_ops;
    tty_dev_t *vtty_dev;
    if (tty_register_driver(console_driver, &vtty_dev))
    {
        KERNEL_PANIC(MODULE, "Couldn't register console driver");
    }
    vtty_dev->tty = master_display_fg->vc_tty;
    // kbd_init();
    // console_map_init();
    return KERRNO_SUCCESSES;
}

const dev_class_t vtconsole_class = {
    .name = "vtconsole",
    .device_added = vt_console_added,
    .device_removed = vt_console_removed,
    .type = DEVICE_SERIAL,
};

status_t vt_init()
{
    class_register(&vtconsole_class);

    LOCK();

    conswitchp = (virt_con_ops_t *)&dummy_con;
    const char *name = conswitchp->con_name;

    for (int i = 0; i < MAX_CONSOLE_DRIVER; i++)
    {
        console_driver_t *con_driver = &registered_con_driver[i];

        if (con_driver->con == NULL)
        {
            con_driver->con = conswitchp;
            con_driver->desc = name;
            con_driver->first = 0;
            con_driver->last = MAX_CONSOLES - 1;
            break;
        }
    }

    for (int i = 0; i < MAX_CONSOLES; i++)
    {
        con_driver_map[i] = conswitchp;
    }

    int currcons = 0;
    for (int currcons = 0; currcons < MIN_CONSOLES; currcons++)
    {
        vc_data_t *vc = kzmalloc(sizeof(vc_data_t));

        consoles[currcons].data = vc;
        // INIT_WORK(&vc_cons[currcons].SAK_work, vc_SAK);
        // tty_port_init(&vc->port);
        vc_data_init(vc, currcons, true);
        vc->vc_screenbuf = kzmalloc(vc->vc_screenbuf_cells * sizeof(vt_cell_t));
        vc_init(vc, currcons || !vc->vc_sw->con_save_screen);
    }

    currcons = fg_console = 0;
    vc_data_t *vc = consoles[currcons].data;
    master_display_fg = vc;
    vc_set_origin(vc);
    vc_save_screen(vc);
    vc_gotoxy(vc, vc->vc_x, vc->vc_y);
    update_screen(vc);
    trace_info(MODULE, "Console: %s %s %dx%d", vc_can_color(vc) ? "color" : "mono", name, vc->vc_cols, vc->vc_rows);

#ifdef CONFIG_VT_CONSOLE
    console_register(&vt_console_driver);
#endif

    UNLOCK();

    return KERRNO_SUCCESSES;
}

CONSOLE_INITCALL(vt_init);

INTERNAL int vc_do_bind_con_driver(const virt_con_ops_t *csw, int first, int last, int deflt)
{
    ENTER_FUNC("%p, %u, %u, %s", csw, first, last, deflt BOOL_TO_STRING);
    const char *desc = NULL;
    console_driver_t *con_driver;
    int i;
    int j = -1;
    int k = -1;
    status_t ret = ENODEV;

    WARN_LOCK_UNLOCKED();

    /* check if driver is registered */
    for (i = 0; i < MAX_CONSOLE_DRIVER; i++)
    {
        con_driver = &registered_con_driver[i];

        if (con_driver->con == csw)
        {
            desc = con_driver->desc;
            ret = 0;
            break;
        }
    }

    if (ret)
    {
        return ret;
    }

    if (!(con_driver->flag & CON_DRIVER_FLAG_INIT))
    {
        // csw->con_startup(NULL);
        con_driver->flag |= CON_DRIVER_FLAG_INIT;
    }

    if (deflt)
    {
        conswitchp = (virt_con_ops_t *)csw;
        trace_debug(MODULE, "conswitchp = %p", conswitchp);
    }

    first = max(first, con_driver->first);
    last = min(last, con_driver->last);

    for (i = first; i <= last; i++)
    {
        vc_data_t *vc = consoles[i].data;

        con_driver_map[i] = (virt_con_ops_t *)csw;

        if (!vc || !vc->vc_sw)
        {
            continue;
        }

        j = i;

        if (con_is_visible(vc))
        {
            k = i;
            vc_save_screen(vc);
        }

        bool old_was_color = vc_can_color(vc);

        vc->vc_sw->con_deinit(vc);
        vc->vc_origin = vc->vc_screenbuf;
        vc_data_init(vc, i, true);
        vc_set_origin(vc);
        // update_attr(vc);

        if (old_was_color != vc_can_color(vc))
        {
            vc_clear_buffer_attributes(vc);
        }
    }

    trace_info(MODULE, "Console: switching ");
    if (!deflt)
    {
        trace_info(MODULE, "consoles %d-%d ", first + 1, last + 1);
    }
    if (j >= 0)
    {
        vc_data_t *vc = consoles[j].data;
        // switch_screen(vc);
        // master_display_fg = vc;

        trace_info(MODULE, "to %u %s %s %dx%d", j, vc_can_color(vc) ? "color" : "mono", desc, vc->vc_cols, vc->vc_rows);
        trace_info(MODULE, "switching to %p", vc);

        trace_info(MODULE, "i = %u, j = %u, k = %u", i, j, k);
        trace_debug(MODULE, "conswitchp = %p", conswitchp);
        trace_debug(MODULE, "vc->vc_sw = %p", vc->vc_sw);
        if (k >= 0)
        {
            vc = consoles[k].data;
            update_screen(vc);
        }
    }
    else
    {
        trace_info(MODULE, "to \"%s\"", desc);
    }

    return ret;
};

INTERNAL status_t vc_do_register_con_driver(const virt_con_ops_t *con, uint32_t first, uint32_t last)
{
    ENTER_FUNC("%p, %u, %u", con, first, last);
    WARN_LOCK_UNLOCKED();

    console_driver_t *con_driver = NULL;
    for (int i = 0; i < MAX_CONSOLE_DRIVER; i++)
    {
        con_driver = &registered_con_driver[i];

        /* already registered */
        if (con_driver->con == con)
        {
            KERRNO_RETURN(EBUSY, "console is already registered");
        }
    }

    const char *desc = con->con_name;

    status_t ret = EINVAL;

    for (int i = 0; i < MAX_CONSOLE_DRIVER; i++)
    {
        con_driver = &registered_con_driver[i];

        if (con_driver->con == NULL && !(con_driver->flag & CON_DRIVER_FLAG_ZOMBIE))
        {
            con_driver->con = con;
            con_driver->desc = desc;
            con_driver->node = i;
            con_driver->flag = CON_DRIVER_FLAG_MODULE | CON_DRIVER_FLAG_INIT;
            con_driver->first = first;
            con_driver->last = last;
            ret = KERRNO_SUCCESSES;
            break;
        }
    }

    if (ret)
    {
        LOG_RETURN(ret);
        // return ret;
    }

    con_driver->dev = core_device_create(&vtconsole_class, NULL, con_driver, "vtcon%i", con_driver->node);
    if (!con_driver->dev)
    {
        trace_warn(MODULE, "Unable to create device for %s", con_driver->desc);
        con_driver->dev = NULL;
    }
    else
    {
        con_driver->flag |= CON_DRIVER_FLAG_ATTR;
    }
    return KERRNO_SUCCESSES;
}

status_t vc_register_con_driver(const virt_con_ops_t *con, uint32_t first, uint32_t last)
{
    ENTER_FUNC("%p, %u, %u", con, first, last);
    LOCK();
    status_t ret = vc_do_register_con_driver(con, first, last);
    UNLOCK();
    return ret;
}

status_t vc_take_over_console(const virt_con_ops_t *con, uint32_t first, uint32_t last, bool is_default)
{
    ENTER_FUNC("%p, %u, %u, %s", con, first, last, is_default BOOL_TO_STRING);
    LOCK();
    status_t ret = vc_do_register_con_driver(con, first, last);
    KERRNO_CHECK_RETURN(ret);
    if (ret == EBUSY)
    {
        ret = KERRNO_SUCCESSES;
    }

    if (ret == KERRNO_SUCCESSES)
    {
        ret = vc_do_bind_con_driver(con, first, last, is_default);
        UNLOCK();
        return ret;
    }
    UNLOCK();
    return ret;
}


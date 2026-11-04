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

static virtual_console_data_t *master_display_fg;

static int fg_console;

#define LOCK_NAME vt_lock
DEF_LOCK(LOCK_NAME);

status_t vt_putc(tty_struct_t *tty, wchar_t ch)
{
    ENTER_FUNC("%p, 0x%x", tty, ch);
    virtual_console_data_t *data = tty->tty_dev->driver_data;

    return KERRNO_UNSUCCESS;
}

static tty_ops_t vt_ops = {
    .put_char = vt_putc,
};

static tty_driver_t vt = {
    .name = "tty",
    .ops = &vt_ops,
};

void vc_redraw_screen(virtual_console_data_t *vc, int is_switch);

INTERNAL status_t vt_data_init(virtual_console_data_t *vc, int num, bool init)
{
    ENTER_FUNC("%p, %u, %s", vc, num, init BOOL_TO_STRING);
    trace_debug(MODULE, "conswitchp = %p", conswitchp);
    vc->current = conswitchp;

    vc->current_font = psf_font_builtin();

    if (con_driver_map[num])
    {
        vc->current = con_driver_map[num];
    }

    vc->console_number = num;
    vc->display_fg = &master_display_fg;
    vc->can_do_color = 0;
    status_t ret = vc->current->init(vc, init);
    if (ret == KERRNO_SUCCESSES)
    {
        vc->console_bpr = vc->size.cols << 1;
        vc->buffer_size = (vc->size.rows * vc->size.cols) * sizeof(virtual_console_cell_t);
        return KERRNO_SUCCESSES;
    }
    return ret;
}

static void vt_data_deinit(virtual_console_data_t *vc)
{
    vc->current->uninit(vc);
}

/**
 * vc_is_visible - checks whether the current console is visible
 * @vc: virtual console
 *
 * RETURNS: zero if not visible, nonzero if visible
 */
bool vc_is_visible(const virtual_console_data_t *vc)
{
    return *vc->display_fg == vc;
}

INTERNAL void vc_save_screen(virtual_console_data_t *vc)
{
    if (vc->current->save_screen)
    {
        vc->current->save_screen(vc);
    }
}

INTERNAL void vc_set_origin(virtual_console_data_t *vc)
{
    if (!vc_is_visible(vc) || !vc->current->set_origin || vc->current->set_origin(vc))
    {
        vc->origin = (uint32_64_t)vc->cell_buffer;
    }
    vc->visible_origin = vc->origin;
    vc->scr_end = vc->origin + vc->buffer_size;
    vc->pos = (virtual_console_cell_t *)(vc->origin + vc->console_bpr * vc->state.y + 2 * vc->state.x);
}

INTERNAL ALWAYS_INLINE bool vc_is_fg(virtual_console_data_t *vc)
{
    return vc->console_number == fg_console;
}

INTERNAL ALWAYS_INLINE bool vc_should_update(const virtual_console_data_t *vc)
{
    return vc_is_visible(vc);
}

INTERNAL void vc_putc(virtual_console_data_t *vc, virtual_console_cell_t *ca, uint32_t y, uint32_t x)
{
    if (vc->current->putc)
    {
        vc->current->putc(vc, *ca, y, x);
    }
    else
    {
        vc->current->putcs(vc, ca, 1, y, x);
    }
}

INTERNAL virtual_console_cell_t softcursor_original = {.cp = (~0)};

INTERNAL void vc_add_softcursor(virtual_console_data_t *vc)
{
    virtual_console_cell_t i = *vc->pos;
    uint32_t type = vc->cursor_type;

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
    *vc->pos = i;
    if (vc_should_update(vc))
    {
        vc_putc(vc, &i, vc->state.y, vc->state.x);
    }
}

INTERNAL void vc_hide_softcursor(virtual_console_data_t *vc)
{
    if (softcursor_original.cp != (~0))
    {
        *vc->pos = softcursor_original;
        if (vc_should_update(vc))
        {
            vc_putc(vc, &softcursor_original, vc->state.y, vc->state.x);
        }
        softcursor_original.cp = (~0);
    }
}

INTERNAL void vc_hide_cursor(virtual_console_data_t *vc)
{
    // if (vc_is_sel(vc))
    // {
    //     clear_selection();
    // }

    vc->current->cursor(vc, false);
    vc_hide_softcursor(vc);
}

INTERNAL void vc_set_cursor(virtual_console_data_t *vc)
{
    if (!vc_is_fg(vc) || !vc->text_mode)
    {
        return;
    }
    if (vc->deccm)
    {
        // if (vc_is_sel(vc))
        // {
        //     clear_selection();
        // }
        vc_add_softcursor(vc);
        if (CUR_SIZE(vc->cursor_type) != CUR_NONE)
        {
            vc->current->cursor(vc, true);
        }
    }
    else
    {
        vc_hide_cursor(vc);
    }
}

INTERNAL void vc_gotoxy(virtual_console_data_t *vc, uint32_t new_x, uint32_t new_y)
{

    if (new_x < 0)
    {
        vc->state.x = 0;
    }
    else
    {
        if (new_x >= vc->size.cols)
        {
            vc->state.x = vc->size.cols - 1;
        }
        else
        {
            vc->state.x = new_x;
        }
    }

    uint32_t min_y;
    uint32_t max_y;
    if (vc->decom)
    {
        min_y = vc->top;
        max_y = vc->bottom;
    }
    else
    {
        min_y = 0;
        max_y = vc->size.rows;
    }
    if (new_y < min_y)
    {
        vc->state.y = min_y;
    }
    else if (new_y >= max_y)
    {
        vc->state.y = max_y - 1;
    }
    else
    {
        vc->state.y = new_y;
    }
    vc->pos = (virtual_console_cell_t *)(vc->origin + vc->state.y * vc->console_bpr + (vc->state.x << 1));
    vc->need_wrap = 0;
}

INTERNAL void vc_save_cur(virtual_console_data_t *vc)
{
    memcpy(&vc->saved_state, &vc->state, sizeof(vc->state));
}

INTERNAL void vc_restore_cur(virtual_console_data_t *vc)
{
    memcpy(&vc->state, &vc->saved_state, sizeof(vc->state));

    vc_gotoxy(vc, vc->state.x, vc->state.y);

    // vc->vc_need_wrap = 0;
}

static color_t default_color = 0x00FFFFFF;           /* white */
static color_t default_italic_color = 0x0000AA00;    // green (ASCII)
static color_t default_underline_color = 0x0000AAAA; // cyan (ASCII)

INTERNAL void vc_init(virtual_console_data_t *vc, int do_clear)
{
    int j, k;

    vc_set_origin(vc);
    vc->pos = (virtual_console_cell_t *)vc->origin;
    vc->text_mode = 1;
    vc->def_color = default_color;
    vc->ulcolor = default_underline_color;
    vc->itcolor = default_italic_color;
    vc->halfcolor = 0x808080; /* grey */
    // init_waitqueue_head(&vc->paste_wait);
    // reset_terminal(vc, do_clear);
}

#define VC_MAXCOL (32767)
#define VC_MAXROW (32767)

status_t vc_allocate(uint32_t currcons)
{
    ENTER_FUNC("%u", currcons)
    virtual_console_data_t *vc;
    status_t ret;

    WARN_LOCK_UNLOCKED();

    if (currcons >= MAX_CONSOLES)
    {
        return ENXIO;
    }

    if (consoles[currcons].data)
    {
        return KERRNO_SUCCESSES;
    }

    vc = kzmalloc(sizeof(virtual_console_data_t));
    if (!vc)
    {
        return ENOMEM;
    }

    consoles[currcons].data = vc;
    // tty_port_init(&vc->port);
    // vc->port.ops = &vc_port_ops;
    // INIT_WORK(&vc_cons[currcons].SAK_work, vc_SAK);

    vt_data_init(vc, currcons, true);

    // if (!*vc->uni_pagedict_loc)
    // {
    //     con_set_default_unimap(vc);
    // }

    if (vc->size.cols > VC_MAXCOL || vc->size.rows > VC_MAXROW || !vc->buffer_size)
    {
        ret = EINVAL;
        KERRNO_NO_RETURN(EINVAL, "Size is invalid");
        trace_debug(MODULE, "vc->size = { %ux%u }, vc->buffer_size = %u", vc->size.cols, vc->size.rows, vc->buffer_size);
        goto err_free;
    }

    vc->cell_buffer = kzmalloc(vc->buffer_size);

    if (!vc->cell_buffer)
    {
        ret = ENOMEM;
        KERRNO_NO_RETURN(ENOMEM, "");
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

INTERNAL ALWAYS_INLINE status_t vc_resize_screen(virtual_console_data_t *vc, int width, int height, bool from_user)
{
    ENTER_FUNC("%p, %u, %u, %s", vc, width, height, from_user BOOL_TO_STRING);
    /* Resizes the resolution of the display adapater */
    int err = 0;

    if (vc->current->resize)
    {
        err = vc->current->resize(vc, width, height, from_user);
    }

    return err;
}

INTERNAL status_t vc_do_resize(tty_struct_t *tty, virtual_console_data_t *vc, uint32_t cols, uint32_t lines, bool from_user)
{
    ENTER_FUNC("%p, %p, %u, %u, %s", tty, vc, cols, lines, from_user BOOL_TO_STRING);
    status_t err = 0;

    WARN_LOCK_UNLOCKED();

    if (cols > VC_MAXCOL || lines > VC_MAXROW)
    {
        KERRNO_RETURN(EINVAL, "");
    }

    uint32_t new_cols = (cols ? cols : vc->size.cols);
    uint32_t new_rows = (lines ? lines : vc->size.rows);
    size_t new_row_size = new_cols << 1;
    size_t new_screen_size = new_rows * new_cols * sizeof(virtual_console_cell_t);

    if (new_cols == vc->size.cols && new_rows == vc->size.rows)
    {
        return vc_resize_screen(vc, new_cols, new_rows, from_user);
    }

    if (!new_screen_size)
    {
        KERRNO_RETURN(EINVAL, "");
    }
    virtual_console_cell_t *newscreen = kzmalloc(new_screen_size);
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

    uint32_t old_rows = vc->size.rows;
    size_t old_row_size = vc->console_bpr;

    err = vc_resize_screen(vc, new_cols, new_rows, from_user);
    if (err)
    {
        kfree(newscreen);
        // vc_uniscr_free(new_uniscr);
        return err;
    }

    vc->size.rows = new_rows;
    vc->size.cols = new_cols;
    vc->console_bpr = new_row_size;
    vc->buffer_size = new_screen_size;

    uint32_64_t rlth = min(old_row_size, new_row_size);
    uint32_64_t rrem = new_row_size - rlth;
    vaddr_t old_origin = vc->origin;
    vaddr_t new_origin = (vaddr_t)newscreen;
    vaddr_t new_scr_end = new_origin + new_screen_size;

    uint32_t first_copied_row = 0;
    if (vc->state.y > new_rows)
    {
        if (old_rows - vc->state.y < new_rows)
        {
            first_copied_row = (old_rows - new_rows);
        }
        else
        {
            first_copied_row = (vc->state.y - new_rows / 2);
        }
        old_origin += first_copied_row * old_row_size;
    }
    else
    {
        first_copied_row = 0;
    }
    vaddr_t end = old_origin + old_row_size * min(old_rows, new_rows);

    // vc_uniscr_copy_area(new_uniscr, new_cols, new_rows, vc->vc_uni_lines, rlth / 2, first_copied_row, min(old_rows, new_rows));
    // vc_uniscr_set(vc, new_uniscr);

    // vc_update_attr(vc);

    trace_debug(MODULE, "newscreen = %p", newscreen);
    trace_debug(MODULE, "new_origin = %p, old_origin = %p, rlth = %u", new_origin, old_origin, rlth);
    trace_debug(MODULE, "new_row_size = %u, old_row_size = %u, rrem = %u", new_row_size, old_row_size, rrem);
    while (old_origin < end)
    {
        memcpy((virtual_console_cell_t *)new_origin, (virtual_console_cell_t *)old_origin, rlth * sizeof(virtual_console_cell_t));
        if (rrem)
        {
            memset((void *)(new_origin + rlth * sizeof(virtual_console_cell_t)), 0, rrem * sizeof(virtual_console_cell_t));
        }
        old_origin += old_row_size;
        new_origin += new_row_size;
    }
    trace_debug(MODULE, "new_origin = %p, new_scr_end = %p, old_origin = %p, new_scr_end - new_origin = %p", new_origin, new_scr_end, old_origin, new_scr_end - new_origin);
    if (new_scr_end > new_origin)
    {
        memset((void *)new_origin, 0, (new_scr_end - new_origin));
    }
    virtual_console_cell_t *oldscreen = vc->cell_buffer;
    vc->cell_buffer = newscreen;
    vc->buffer_size = new_screen_size;
    vc_set_origin(vc);
    trace_debug(MODULE, "oldscreen = %p", oldscreen);
    kfree(oldscreen);

    /* do part of a reset_terminal() */
    vc->top = 0;
    vc->bottom = vc->size.rows;
    vc_gotoxy(vc, vc->state.x, vc->state.y);
    vc_save_cur(vc);

    if (tty)
    {
        /* Rewrite the requested winsize data with the actual
           resulting sizes */
        winsize_t ws;
        memset(&ws, 0, sizeof(ws));
        ws.ws_row = vc->size.rows;
        ws.ws_col = vc->size.cols;
        ws.ws_ypixel = vc->console_bpr;
        tty_do_resize(tty, &ws);
    }

    if (vc_is_visible(vc))
    {
        update_screen(vc);
    }
    // vt_event_post(VT_EVENT_RESIZE, vc->vc_num, vc->vc_num);
    // notify_update(vc);
    return err;
}

status_t vc_resize(virtual_console_data_t *vc, uint32_t cols, uint32_t rows, bool from_user)
{
    ENTER_FUNC("%p, %u, %u, %s", vc, cols, rows, from_user BOOL_TO_STRING);
    tty_struct_t *tty = vc->tty;
    // tty_dev_t *tty_dev = tty->tty_dev;
    // LOCK();
    status_t ret = vc_do_resize(tty, vc, cols, rows, from_user);
    // UNLOCK();
    return ret;
}

static status_t vt_resize(tty_struct_t *tty, winsize_t *ws)
{
    virtual_console_data_t *vc = tty->tty_dev->driver_data;

    // guard(console_lock)();
    return vc_do_resize(tty, vc, ws->ws_col, ws->ws_row, false);
}

void vc_clear_buffer_attributes(virtual_console_data_t *vc)
{
    virtual_console_cell_t *p = (virtual_console_cell_t *)vc->origin;
    int count = vc->buffer_size / sizeof(virtual_console_cell_t);
    // int mask = vc->vc_hi_font_mask | 0xFF;

    for (; count > 0; count--, p++)
    {
        p->bg = p->fg = 0;
        p->cp = ' ';
        // scr_writew((scr_readw(p) & mask) | (vc->vc_video_erase_char & ~mask), p);
    }
}

void vc_redraw_screen(virtual_console_data_t *vc, int is_switch)
{
    int redraw = 0;

    if (!vc)
    {
        /* strange ... */
        /* printk("vc_redraw_screen: tty %d not allocated ??\n", new_console+1); */
        return;
    }

    if (is_switch)
    {
        virtual_console_data_t *old_vc = consoles[fg_console].data;
        if (old_vc == vc)
        {
            return;
        }
        if (!vc_is_visible(vc))
        {
            redraw = 1;
        }
        *vc->display_fg = vc;
        fg_console = vc->console_number;
        vc_hide_cursor(old_vc);
        if (!vc_is_visible(old_vc))
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

    if (redraw)
    {
        bool update;
        int old_was_color = vc->can_do_color;

        vc_set_origin(vc);
        update = vc->current->switch_console(vc);
        // set_palette(vc);
        /*
         * If console changed from mono<->color, the best we can do
         * is to clear the buffer attributes. As it currently stands,
         * rebuilding new attributes from the old buffer is not doable
         * without overly complex code.
         */
        if (old_was_color != vc->can_do_color)
        {
            // update_attr(vc);
            vc_clear_buffer_attributes(vc);
        }

        if (update && !vc->text_mode)
        {
            FUNC_NOT_IMPLEMENTED();
            // do_update_region(vc, vc->origin, vc->buffer_size / 2);
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
    virtual_console_cell_t *from;
    virtual_console_cell_t *to;
    int x;
} vc_draw_region_t;

static void vt_con_flush(virtual_console_data_t *vc, vc_draw_region_t *draw)
{
    ENTER_FUNC("%p, %p", vc, draw);
    trace_debug(MODULE, "draw->x = %u", draw->x);
    trace_debug(MODULE, "draw->to = %p", draw->to);
    trace_debug(MODULE, "draw->from = %p", draw->from);
    if (draw->x < 0)
    {
        // return;
    }

    vc->current->putcs(vc, draw->from, draw->to - draw->from, draw->x, vc->state.y);
    draw->x = -1;
}

INTERNAL status_t vc_con_write_normal(virtual_console_data_t *vc, wchar_t c)
{
    ENTER_FUNC("%p, 0x%x", vc, c);

    status_t ret = font_has_glyph(vc->current_font, c);
    if (ret)
    {
        return ret; /* nothing to display */
    }

    vc->pos->cp = c;
    vc->pos->bg = 0x0;
    vc->pos->fg = 0x00FFFFFF;
    vc->pos++;

    return KERRNO_SUCCESSES;
}

status_t vc_con_install(tty_driver_t *driver, tty_struct_t *tty)
{
    ENTER_FUNC("%p, %p", driver, tty);
    unsigned int currcons = tty->index;
    virtual_console_data_t *vc;
    int ret;

    if (spinlock_is_locked(&SPIN_NAME(LOCK_NAME)))
    {
        trace_warn(MODULE, "lock is taken");
    }

    WARN_LOCK_UNLOCKED();
    LOCK();
    ret = vc_allocate(currcons);
    if (ret)
    {
        UNLOCK();
        return ret;
    }

    vc = consoles[currcons].data;

    if (vc->tty)
    {
        FUNC_NOT_IMPLEMENTED();
    }

    tty->tty_dev->driver_data = vc;
    vc->tty = tty;

    if (!tty->winsize.ws_row && !tty->winsize.ws_col)
    {
        tty->winsize.ws_row = consoles[currcons].data->size.rows;
        tty->winsize.ws_col = consoles[currcons].data->size.cols;
    }
    if (vc->vc_utf)
    {
        tty->termios.c_iflag |= IUTF8;
    }
    else
    {
        tty->termios.c_iflag &= ~IUTF8;
    }

    UNLOCK();

    return 0;
}

static status_t vt_do_con_write(tty_struct_t *tty, const wchar_t *buf, int count, size_t *size_out)
{
    ENTER_FUNC("%p, %p, %u, %p", tty, buf, count, size_out);
    virtual_console_data_t *vc = tty->tty_dev->driver_data;
    vc_draw_region_t draw = {
        .x = vc->state.x,
        .from = vc->pos,
    };
    int n = 0;
    unsigned int currcons;
    bool rescan;

    LOCK();
    currcons = vc->console_number;
    if (!vc_cons_allocated(currcons))
    {
        /* could this happen? */
        trace_info(MODULE, "con_write: tty %d not allocated", currcons + 1);
        UNLOCK();
        return 0;
    }

    /* undraw cursor first */
    if (vc_is_fg(vc))
    {
        vc_hide_cursor(vc);
    }

    while (count)
    {
        wchar_t orig = *buf;
        buf++;
        n++;
        count--;
        // rescan_last_byte:
        //         c = orig;
        //         rescan = false;

        //         tc = vc_translate(vc, &c, &rescan);
        //         if (tc == -1)
        //         {
        //             continue;
        //         }

        //         param.c = tc;
        //         if (atomic_notifier_call_chain(&vt_notifier_list, VT_PREWRITE,
        //                                        &param) == NOTIFY_STOP)
        //         {
        //             continue;
        //         }

        //         if (vc_is_control(vc, tc, c))
        //         {
        //             vt_con_flush(vc, &draw);
        //             do_con_trol(tty, vc, orig);
        //             continue;
        //         }

        if (vc_con_write_normal(vc, orig))
        {
            continue;
        }

        //         if (rescan)
        //         {
        //             goto rescan_last_byte;
        //         }
    }
    draw.to = draw.from + n;
    vt_con_flush(vc, &draw);
    // notify_update(vc);

    *size_out = n;
    UNLOCK();
    return KERRNO_SUCCESSES;
}

status_t con_get_index(tty_driver_t *tty_driver, int *index)
{
    *index = fg_console;
    return KERRNO_SUCCESSES;
}

status_t con_put_char(tty_struct_t *tty, wchar_t ch)
{
    size_t si;
    return vt_do_con_write(tty, &ch, 1, &si);
}

status_t vc_con_write(tty_struct_t *tty, const wchar_t *buf, size_t count, ssize_t *size_out)
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
    //     start_x = vc->state.x;
    //     cnt = 0;
    //     while (count--)
    //     {
    //         c = *b++;
    //         if (c == ASCII_LINEFEED || c == ASCII_CAR_RET ||
    //             c == ASCII_BACKSPACE || vc->vc_need_wrap)
    //         {
    //             if (cnt && con_is_visible(vc))
    //             {
    //                 vc->vc_sw->con_putcs(vc, start, cnt, vc->state.y, start_x);
    //             }
    //             cnt = 0;
    //             if (c == ASCII_BACKSPACE)
    //             {
    //                 bs(vc);
    //                 start = (ushort *)vc->vc_pos;
    //                 start_x = vc->state.x;
    //                 continue;
    //             }
    //             if (c != ASCII_CAR_RET)
    //             {
    //                 lf(vc);
    //             }
    //             cr(vc);
    //             start = (ushort *)vc->vc_pos;
    //             start_x = vc->state.x;
    //             if (c == ASCII_LINEFEED || c == ASCII_CAR_RET)
    //             {
    //                 continue;
    //             }
    //         }
    //         vc_uniscr_putc(vc, c);
    //         scr_writew((vc->vc_attr << 8) + c, (unsigned short *)vc->vc_pos);
    //         notify_write(vc, c);
    //         cnt++;
    //         if (vc->state.x == vc->vc_cols - 1)
    //         {
    //             vc->vc_need_wrap = 1;
    //         }
    //         else
    //         {
    //             vc->vc_pos += 2;
    //             vc->state.x++;
    //         }
    //     }
    //     if (cnt && con_is_visible(vc))
    //     {
    //         vc->vc_sw->con_putcs(vc, start, cnt, vc->state.y, start_x);
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
    .get_index = con_get_index,
    .install = vc_con_install,
    // 	.open = con_open,
    // 	.close = con_close,
    .write = vc_con_write,
    // .write_room = con_write_room,
    .put_char = con_put_char,
    // 	.flush_chars = con_flush_chars,
    // 	.ioctl = vt_ioctl,
    // #ifdef CONFIG_COMPAT
    // 	.compat_ioctl = vt_compat_ioctl,
    // #endif
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
    if (tty_register_driver(console_driver))
    {
        KERNEL_PANIC(MODULE, "Couldn't register console driver");
    }
    // kbd_init();
    // console_map_init();
    return KERRNO_SUCCESSES;
}

static const dev_class_t vtconsole_class = {
    .name = "vtconsole",
    .device_added = vt_console_added,
    .device_removed = vt_console_removed,
    .type = DEVICE_SERIAL,
};

status_t vt_init()
{
    class_register(&vtconsole_class);

    conswitchp = (virt_con_ops_t *)&dummy_con;
    const char *name;
    conswitchp->startup(&name);

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

    for (int currcons = 0; currcons < MIN_CONSOLES; currcons++)
    {
        virtual_console_data_t *vc = kzmalloc(sizeof(virtual_console_data_t));
        
        consoles[currcons].data = vc;
        // INIT_WORK(&vc_cons[currcons].SAK_work, vc_SAK);
        // tty_port_init(&vc->port);
        vt_data_init(vc, currcons, true);
        /* Assuming vc->vc_{cols,rows,screenbuf_size} are sane here. */
        vc->cell_buffer = kzmalloc(vc->buffer_size);
        vc_init(vc, currcons || !vc->current->save_screen);
    }

    virtual_console_data_t *vc = consoles[0].data;
    fg_console = 0;
    master_display_fg = vc;
    vc_set_origin(vc);
    vc_save_screen(vc);
    vc_gotoxy(vc, vc->state.x, vc->state.y);
    update_screen(vc);
    trace_info(MODULE, "Console: %s %s %dx%d\n", vc->can_do_color ? "color" : "mono", name, vc->size.cols, vc->size.rows);

#ifdef CONFIG_VT_CONSOLE
    console_register(&vt_console_driver);
#endif

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
        csw->startup(NULL);
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
        virtual_console_data_t *vc = consoles[i].data;

        con_driver_map[i] = (virt_con_ops_t *)csw;

        if (!vc || !vc->current)
        {
            continue;
        }

        j = i;

        if (vc_is_visible(vc))
        {
            k = i;
            vc_save_screen(vc);
        }

        vc->current->uninit(vc);
        vc->origin = (vaddr_t)vc->cell_buffer;
        vc_init(vc, i);
        vc_set_origin(vc);
    }

    trace_info(MODULE, "Console: switching ");
    if (!deflt)
    {
        trace_info(MODULE, "consoles %d-%d ", first + 1, last + 1);
    }
    if (j >= 0)
    {
        virtual_console_data_t *vc = consoles[j].data;

        trace_info(MODULE, "to %s %s %dx%d", vc->can_do_color ? "colour" : "mono", desc, vc->size.cols, vc->size.rows);

        if (k >= 0)
        {
            vc = consoles[k].data;
            update_screen(vc);
        }
    }
    else
    {
        trace_info(MODULE, "to %s\n", desc);
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

    const char *desc;
    status_t ret = con->startup(&desc);
    if (ret != KERRNO_SUCCESSES || !desc)
    {
        KERRNO_RETURN(ENODEV, "");
    }

    ret = EINVAL;

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
        return ret;
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


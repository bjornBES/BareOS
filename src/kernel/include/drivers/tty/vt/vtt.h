/*
 * File: vtt.h
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/video/console/kernel_fontt.h"
#include "drivers/tty/ttyt.h"
#include "dev/devicet.h"

#include <types.h>

#define MAX_CONSOLE_DRIVER          8

#define MIN_CONSOLES                1
#define MAX_CONSOLES                8

#define CUR_MAKE(size, change, set) ((size) | ((change) << 8) | ((set) << 16))
#define CUR_SIZE(c)                 ((c) & 0x00000F)
#define CUR_DEF                     0
#define CUR_NONE                    1
#define CUR_UNDERLINE               2
#define CUR_LOWER_THIRD             3
#define CUR_LOWER_HALF              4
#define CUR_TWO_THIRDS              5
#define CUR_BLOCK                   6
#define CUR_SW                      0x000010
#define CUR_ALWAYS_BG               0x000020
#define CUR_INVERT_FG_BG            0x000040
#define CUR_FG                      0xAAAAAA
#define CUR_BG                      0xAAAAAA
#define CUR_CHANGE(c)               ((c) & 0x00FF00)
#define CUR_SET(c)                  (((c) & 0xFF0000) >> 8)

#define VC_TABSTOPS_COUNT           256u

typedef struct vt_cell
{
    code_point_t cp; // glyph index
    color_t fg;      // RGBX8888
    color_t bg;      // RGBX8888
    uint32_t attr;
} vt_cell_t;

typedef struct virtual_console_size
{
    uint32_t cols;
    uint32_t rows;
} virtual_console_size_t;

typedef struct vc_state
{
    uint32_t x;
    uint32_t y;

    color_t color;

    /* attribute flags */
    // enum vc_intensity intensity;
    // bool italic;
    // bool underline;
    // bool blink;
    // bool reverse;
} vc_state_t;

typedef struct vt_damage
{
    uint16_t x0;
    uint16_t x1;
} vt_damage_t;

typedef enum vc_flags
{
    VC_FLAGS_UTF8,
    VC_FLAGS_COLOR,
    VC_FLAGS_TEXT,

    VC_FLAG_COUNT
} vc_flags_t;

typedef enum vc_mode
{
    VC_MODE_SCREEN_MODE,
    VC_MODE_ORIGIN_MODE,
    VC_MODE_AUTOWRAP_MODE,
    VC_MODE_INSERT_MODE,
    VC_MODE_CUR_VISIBLE,
    VC_MODE_NEED_WRAP,
} vc_mode_t;

typedef struct virt_con_ops virt_con_ops_t;

typedef struct vc_data
{
    int vc_index;
    tty_struct_t *vc_tty;
    virt_con_ops_t *vc_sw;
    void *vc_priv;

    uint16_t vc_cols;
    uint16_t vc_rows;
    vt_cell_t *vc_screenbuf;
    size_t vc_screenbuf_cells;

    uint16_t vc_x;
    uint16_t vc_y;
    uint16_t vc_top;    /* Scrolling region */
    uint16_t vc_bottom; /* Scrolling region */
    uint16_t vc_saved_x;
    uint16_t vc_saved_y;
    vt_cell_t *vc_pos;
    vt_cell_t *vc_origin;
    vt_cell_t *vc_scr_end;
    vt_cell_t *vc_visible_origin;
    // uint32_t vc_size_row;

    color_t vc_fg;
    color_t vc_bg;
    uint32_t vc_attr;
    color_t vc_def_fg;
    color_t vc_def_bg;
    color_t vc_saved_fg;
    color_t vc_saved_bg;
    uint32_t vc_saved_attr;
    color_t vc_palette[16];

    uint8_t vc_state;
    uint8_t vc_npar;
    uint32_t vc_par[16];
    uint8_t vc_intermediate;

    uint32_t vc_modes;
    DECLARE_BITMAP(vc_tab_stop, VC_TABSTOPS_COUNT);

    unsigned char vc_utf_count;
    code_point_t vc_utf_char;

    const font_t *current_font;

    vt_damage_t *vc_damage;
    bool vc_visible;

    struct vc_data **vc_display_fg;

    spinlock_t vc_lock;

    device_t *dev;

} vc_data_t;

typedef struct virtual_console
{
    vc_data_t *data;
} virtual_console_t;

typedef struct virt_con_ops
{
    const char *con_name;
    uint32_t con_flags;

    status_t (*con_init)(vc_data_t *vc, bool do_init);
    status_t (*con_deinit)(vc_data_t *vc);

    status_t (*con_resize)(vc_data_t *vc, uint32_t width, uint32_t height, bool from_user);

    status_t (*con_putcs)(vc_data_t *vc, const vt_cell_t *s, size_t count, uint32_t xpos, uint32_t ypos);
    status_t (*con_putc)(vc_data_t *vc, vt_cell_t cell, uint32_t x, uint32_t y);

    status_t (*con_clear)(vc_data_t *vc, const vt_cell_t *blank, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

    status_t (*con_scroll)(vc_data_t *vc, uint32_t top, uint32_t bottom, int dir, uint16_t lines);

    status_t (*con_cursor)(vc_data_t *vc, bool enable);

    status_t (*con_switch)(vc_data_t *vc);

    status_t (*con_set_origin)(vc_data_t *vc);
    status_t (*con_save_screen)(vc_data_t *vc);
} virt_con_ops_t;


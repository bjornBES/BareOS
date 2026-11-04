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


#define MAX_CONSOLE_DRIVER 8

#define MIN_CONSOLES       1
#define MAX_CONSOLES       8

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

typedef struct virtual_console_cell
{
    uint32_t cp; // glyph index
    color_t fg;  // palette indexes
    color_t bg;  // palette indexes
} virtual_console_cell_t;

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

typedef struct virt_con_ops virt_con_ops_t;

typedef struct virtual_console_data
{
    vc_state_t state;
    vc_state_t saved_state;

    tty_struct_t *tty;
    device_t *dev;

    int console_number;
    virtual_console_size_t size;

    uint32_t console_bpr; // bpr = bytes per row

    vaddr_t origin;
    uint32_64_t scr_end;
    uint32_64_t visible_origin;
    uint32_t top;    /* Scrolling region */
    uint32_t bottom; /* Scrolling region */

    const virt_con_ops_t *current;
    struct virtual_console_data **display_fg;

    virtual_console_cell_t *cell_buffer;
    size_t buffer_size;

    color_t def_color; /* Default colors */
    color_t ulcolor;   /* Color for underline mode */
    color_t itcolor;
    color_t halfcolor; /* Color for half intensity mode */

    uint32_t cursor_type;
    uint16_t complement_mask;   /* [#] Xor mask for mouse pointer */
    uint16_t s_complement_mask; /* Saved mouse pointer mask */
    virtual_console_cell_t *pos;

    const font_t *current_font;

    uint32_t disp_ctrl : 1;   /* Display chars < 32? */
    uint32_t toggle_meta : 1; /* Toggle high bit? */
    uint32_t decscnm : 1;     /* Screen Mode */
    uint32_t decom : 1;       /* Origin Mode */
    uint32_t decawm : 1;      /* Autowrap Mode */
    uint32_t deccm : 1;       /* Cursor Visible */
    uint32_t decim : 1;       /* Insert Mode */
    uint32_t vc_utf : 1;

    uint32_t need_wrap : 1;
    uint32_t can_do_color : 1;
    uint32_t text_mode : 1;
    uint32_t unused : 21;

} virtual_console_data_t;

typedef struct virtual_console
{
    virtual_console_data_t *data;
} virtual_console_t;

typedef struct virt_con_ops
{
    status_t (*startup)(const char **name_out);
    status_t (*init)(virtual_console_data_t *vc, bool do_init);
    status_t (*uninit)(virtual_console_data_t *vc);

    status_t (*clear)(virtual_console_data_t *vc, uint32_t xpos, uint32_t ypos, size_t count);

    status_t (*putc)(virtual_console_data_t *vc, virtual_console_cell_t cell, uint32_t x, uint32_t y);
    status_t (*putcs)(virtual_console_data_t *vc, const virtual_console_cell_t *s, size_t count, uint32_t xpos, uint32_t ypos);

    status_t (*cursor)(virtual_console_data_t *vc, bool enable);

    status_t (*switch_console)(virtual_console_data_t *vc);

    status_t (*resize)(virtual_console_data_t *vc, uint32_t width, uint32_t height, bool from_user);

    status_t (*set_origin)(virtual_console_data_t *vc);
    status_t (*save_screen)(virtual_console_data_t *vc);
} virt_con_ops_t;


/*
 * File: dummy_con.c
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/vt/vt.h"

#include "kerrno.h"

#define DUMMY_COLUMNS 80
#define DUMMY_ROWS    25

#define MODULE "dummy-con"

static status_t dummycon_putc(vc_data_t *vc, vt_cell_t c, uint32_t x, uint32_t y)
{
    return KERRNO_SUCCESSES;
}

static status_t dummycon_putcs(vc_data_t *vc, const vt_cell_t *s, size_t count, uint32_t xpos, uint32_t ypos)
{
    return KERRNO_SUCCESSES;
}

static status_t dummycon_switch(vc_data_t *vc)
{
    return KERRNO_UNSUCCESS;
}

static status_t dummycon_init(vc_data_t *vc, bool init)
{
    vc_set_can_color(vc);
    if (init)
    {
        vc->vc_cols = DUMMY_COLUMNS;
        vc->vc_rows = DUMMY_ROWS;
    }
    else
    {
        // vc_resize(vc, DUMMY_COLUMNS, DUMMY_ROWS);
    }
    return KERRNO_SUCCESSES;
}

static status_t dummycon_deinit(vc_data_t *vc)
{
    ENTER_FUNC("%p", vc);
    return KERRNO_SUCCESSES;
}

static status_t dummycon_clear(vc_data_t *vc, const vt_cell_t *blank, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    return KERRNO_SUCCESSES;
}

static status_t dummycon_cursor(vc_data_t *vc, bool enable)
{
    return KERRNO_SUCCESSES;
}

const virt_con_ops_t dummy_con = {
    .con_name = "dummy device",
    .con_flags = 0,
    .con_init = dummycon_init,
    .con_deinit = dummycon_deinit,
    .con_clear = dummycon_clear,
    .con_putc = dummycon_putc,
    .con_putcs = dummycon_putcs,
    .con_cursor = dummycon_cursor,
    // .scroll =	dummycon_scroll,
    .con_switch = dummycon_switch,
    // .blank =	dummycon_blank,
};

/*
 * File: dummy_con.c
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/vt/vtt.h"

#include "kerrno.h"

#define DUMMY_COLUMNS 80
#define DUMMY_ROWS    25

static status_t dummycon_putc(virtual_console_data_t *vc, virtual_console_cell_t c, uint32_t x, uint32_t y)
{
    return KERRNO_SUCCESSES;
}

static status_t dummycon_putcs(virtual_console_data_t *vc, const virtual_console_cell_t *s, size_t count, uint32_t xpos, uint32_t ypos)
{
    return KERRNO_SUCCESSES;
}

static status_t dummycon_switch(virtual_console_data_t *vc)
{
    return KERRNO_UNSUCCESS;
}

static status_t dummycon_startup(const char **name_out)
{
    *name_out = "dummy device";
    return KERRNO_SUCCESSES;
}

static status_t dummycon_init(virtual_console_data_t *vc, bool init)
{
    vc->can_do_color = 1;
    if (init)
    {
        vc->size.cols = DUMMY_COLUMNS;
        vc->size.rows = DUMMY_ROWS;
    }
    else
    {
        // vc_resize(vc, DUMMY_COLUMNS, DUMMY_ROWS);
    }
    return KERRNO_SUCCESSES;
}

static status_t dummycon_deinit(virtual_console_data_t *vc)
{
    return KERRNO_SUCCESSES;
}

static status_t dummycon_clear(virtual_console_data_t *vc, uint32_t xpos, uint32_t ypos, size_t count)
{
    return KERRNO_SUCCESSES;
}

static status_t dummycon_cursor(virtual_console_data_t *vc, bool enable)
{
    return KERRNO_SUCCESSES;
}

const virt_con_ops_t dummy_con = {
    .startup = dummycon_startup,
    .init = dummycon_init,
    .uninit = dummycon_deinit,
    .clear = dummycon_clear,
    .putc = dummycon_putc,
    .putcs = dummycon_putcs,
    .cursor = dummycon_cursor,
    // .scroll =	dummycon_scroll,
    .switch_console = dummycon_switch,
    // .blank =	dummycon_blank,
};

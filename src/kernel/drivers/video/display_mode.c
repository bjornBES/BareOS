/*
 * File: display_mode.c
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/display.h"

#include "sync/spinlock.h"

#include "kerrno.h"

spinlock_t mode_lock = {0};

static display_mode_t cur_mode = {.w = 1280, .h = 720, .bpp = 32};

INTERNAL status_t apply_mode(display_device_t *dev, const display_mode_t *m)
{
    if (!dev->driver->set_mode)
    {
        // firmware fb: keeps its native mode, not an error
        return KERRNO_SUCCESSES;
    }
    return dev->driver->set_mode(dev, m);
}

status_t display_set_mode(display_device_t *dev, const display_mode_t *m)
{
    spinlock_acquire(&mode_lock);
    display_mode_t old = cur_mode; // copy before touching anything
    cur_mode = *m;
    status_t r = apply_mode(dev, &cur_mode);
    if (r != KERRNO_SUCCESSES)
    {
        cur_mode = old;
        r = apply_mode(dev, &old); // best effort: put the hardware back too
        spinlock_release(&mode_lock);
        return r;
    }
    spinlock_release(&mode_lock);
    return display_notify_listeners(dev);
}

status_t display_set_default_mode(display_device_t *dev)
{
    return display_set_mode(dev, &cur_mode);
}

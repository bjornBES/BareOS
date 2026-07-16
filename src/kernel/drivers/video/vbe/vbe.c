/*
 * File: vbe.c
 * File Created: 11 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/video_types.h"
#include "task/threading/spinlock/spinlock.h"

#include "kernel/memory.h"
#include "kernel.h"

typedef struct
{
    spinlock_t lock; // for concurrent access
} vbe_priv_t;

void vbe_shutdown(video_backend_t *backend)
{
    free(backend->priv);
}

void vbe_fill_rect(video_backend_t *backend, uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color)
{
    vbe_priv_t *priv = backend->priv;
    spinlock_acquire(&priv->lock);
    fb_info_t *fb = &backend->fb;
    for (uint32_t row = y; row < y + h; row++)
    {
        uint32_t *line = (uint32_t *)((uint8_t *)fb->addr + row * fb->pitch + x * (fb->bpp / 8));
        memset32(line, color, w);
    }
    spinlock_release(&priv->lock);
}

void vbe_put_pixel(fb_info_t *fb, uint32_t x, uint32_t y, color_t color)
{
    uint32_t *pixel = (uint32_t *)((uint8_t *)fb->addr + y * fb->pitch + x * (fb->bpp / 8));
    *pixel = color;
}

void vbe_set_pixel(video_backend_t *backend, uint32_t x, uint32_t y, color_t color)
{
    vbe_priv_t *priv = backend->priv;
    spinlock_acquire(&priv->lock);

    fb_info_t *fb = &backend->fb;
    vbe_put_pixel(fb, x, y, color);

    spinlock_release(&priv->lock);
}

int vbe_init(video_backend_t *backend)
{
    vbe_priv_t *priv = malloc(sizeof(vbe_priv_t));
    backend->priv = priv;
    return RETURN_GOOD;
}

static video_ops_t vbe_ops = {
    .init = vbe_init,
    .set_pixel = vbe_set_pixel,
    .fill_rect = vbe_fill_rect,
    .flip = NULL, // no double buffer yet
    .shutdown = vbe_shutdown,
};

video_ops_t *vbe_get_ops()
{
    vbe_ops.init = vbe_init;
    vbe_ops.set_pixel = vbe_set_pixel;
    vbe_ops.fill_rect = vbe_fill_rect;
    vbe_ops.shutdown = vbe_shutdown;
    return &vbe_ops;
}

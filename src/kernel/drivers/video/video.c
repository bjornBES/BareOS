/*
 * File: video.c
 * File Created: 27 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

/*
 * File: video.c
 * File Created: 27 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

// Graphical Interface

#include "video.h"
#include "video_types.h"
#include "video_ioctl.h"
#include "video_color.h"
#include "video_text.h"
#include "video_common.h"

#include "vga/vga_text_device.h"

#include "mm/ioremap/ioremap.h"
#include "mm/memdefs.h"
#include "mm/allocator/memory_allocator.h"

#include "kernel/mmu.h"
#include "kernel/memory.h"

#include "errno/errno.h"

#include "vbe/vbe.h"

#include "stdio.h"
#include <util/arrays.h>
#include <defs.h>

#define MODULE "VIDEO"

int current_mode;
framebuffer_t *video_current_mode_data;
framebuffer_t *modes;
int mode_count;
video_backend_t *active_backend;

extern char default8x16Font;

size_t video_write(void *buf, off_t offset, size_t count, device_t *dev)
{
    video_priv_t *priv = (video_priv_t *)dev->priv;
    spinlock_acquire(&priv->lock);
    fb_info_t *fb = &priv->fb;
    memcpy((uint8_t *)fb->addr + offset, buf, count);
    spinlock_release(&priv->lock);
    return count;
}

int video_ioctl(uint32_t cmd, void *arg, device_t *dev)
{
    ENTER_FUNC(MODULE, "0x%x, %p, %p", cmd, arg, dev);
    video_priv_t *priv = (video_priv_t *)dev->priv;
    switch (cmd)
    {
        case VIDEO_IOCTL_GET_INFO :
            memcpy(arg, &priv->fb, sizeof(fb_info_t));
            return 0;
        case VIDEO_IOCTL_CLEAR :
            memset((uint8_t *)priv->fb.addr, 0, priv->fb.pitch * priv->fb.height);
            return 0;
        default :
            return -EINVAL;
    }
}

// Helper Functions
framebuffer_t *video_get_mode_data(int mode)
{
    for (size_t i = 0; i < mode_count; i++)
    {
        framebuffer_t *element = &modes[i];
        // log_debug("VIDEO", "video mode %d==%d", element->mode, mode);
        if (element->mode == mode)
        {
            // log_debug("VIDEO", "found video mode %d %dx%dx%d fb=%p mode&=%p", i, element->width, element->height, element->bpp, element->frame_buffer, element);
            return &modes[i];
        }
    }
    return NULL;
}

void video_set_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = video_pack_color(&active_backend->fb, r, g, b);
    active_backend->ops->set_pixel(active_backend, x, y, color);
}

void video_init(boot_params_t *bp)
{
    log_debug(MODULE, "video_init(%p)", bp);
    mode_count = bp->video.count;
    modes = (framebuffer_t *)kcalloc(mode_count, sizeof(framebuffer_t));
    memcpy(modes, bp->video.entries, sizeof(framebuffer_t) * mode_count);

    video_ops_t *backend_drivers[] = {
        vga_get_ops(),
        vbe_get_ops(),
    };

    active_backend = malloc(sizeof(video_backend_t));

    framebuffer_t *video_current_mode_data = video_get_mode_data(bp->current_mode);
    paddr_t fb = (paddr_t)video_current_mode_data->addr;
    size_t fb_size = (size_t)video_current_mode_data->pitch * video_current_mode_data->height;
    log_debug(MODULE, "pitch (%u) * height (%u) = %u", video_current_mode_data->pitch, video_current_mode_data->height, video_current_mode_data->pitch * video_current_mode_data->height);
    log_debug(MODULE, "width: %u, height: %u pitch: %u, bpp: %u", video_current_mode_data->width, video_current_mode_data->height, video_current_mode_data->pitch, video_current_mode_data->bpp);
    log_debug(MODULE, "paging p%p mapped", fb);
    vaddr_t fb_virt = ioremap(fb, fb_size + PAGE_SIZE);

    video_priv_t *priv = malloc(sizeof(video_priv_t));
    priv->fb.addr = (void *)fb_virt;
    priv->fb.width = video_current_mode_data->width;
    priv->fb.height = video_current_mode_data->height;
    priv->fb.pitch = video_current_mode_data->pitch;
    priv->fb.bpp = video_current_mode_data->bpp;
    priv->fb.red_shift = video_current_mode_data->red_shift;
    priv->fb.green_shift = video_current_mode_data->green_shift;
    priv->fb.blue_shift = video_current_mode_data->blue_shift;
    priv->fb.fmt = FB_PIXEL_BITMASK; // VBE path
    active_backend->fb = priv->fb;

    for (size_t i = 0; i < mode_count; i++)
    {
        modes[i].addr = (uint32_64)fb_virt;
    }
    log_debug(MODULE, "paging %p mapped to virt %p/phys %p", fb, fb_virt, fb);

    device_t *video_dev = malloc(sizeof(device_t));
    video_dev->class_name = "fb";
    video_dev->type = DEVICE_VIDEO;
    video_dev->priv = priv;
    video_dev->write = video_write;
    video_dev->read = NULL; // no read from framebuffer
    video_dev->ioctl = video_ioctl;
    device_register(video_dev);

    for (int i = 0; i < ARRAY_SIZE(backend_drivers); i++)
    {
        log_debug(MODULE, "driver %u, init @ %p", i, backend_drivers[i]->init);
        if (backend_drivers[i]->init(active_backend) == RETURN_GOOD)
        {
            active_backend->ops = backend_drivers[i];
            break;
        }
    }

    video_text_load_font_from_data((void *)&default8x16Font);
}

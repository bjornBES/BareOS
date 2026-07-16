/*
 * File: vga_text_device.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vga_text_device.h"
#include "drivers/video/vbe/vbe_vga.h"
#include "drivers/video/video_text.h"
#include "drivers/video/video_color.h"
#include "drivers/video/video_common.h"
#include "drivers/video/video_ioctl.h"
#include "device/device.h"

#include "drivers/IO/tty/tty_types.h"

#include "debug/debug.h"

#include "kernel/io.h"
#include "kernel/string.h"
#include "kernel.h"

#include "errno/errno.h"

#include <defs.h>

typedef struct
{
    uint8_t extra_glyphs;
    char header[4];
    uint8_t res;
    uint8_t width;
    uint8_t height;
    uint16_t number_of_glyphs;
    uint8_t res2[6];
} PACKED font_struct;

fb_info_t fb;

uint32_t screen_width;
uint32_t screen_height;
uint32_t default_color;
int screen_x, screen_y;
uint8_t glyph_width;
uint8_t glyph_height;
uint16_t glyph_number;

device_t *video_driver;

#define MODULE "VGA"

typedef struct
{
    spinlock_t lock; // for concurrent access
} vga_priv_t;

void vga_shutdown(video_backend_t *backend)
{
    free(backend->priv);
}

void vga_fill_rect(video_backend_t *backend, uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color)
{
}

void vga_set_pixel(video_backend_t *backend, uint32_t x, uint32_t y, color_t color)
{
}

void vga_hexdump(void *ptr, int len)
{
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < len; ++i)
    {
        if ((i & 0xF) == 0)
        {
            fprintf(VFS_FD_DEBUG, "\n%02x: ", i);
        }
        fprintf(VFS_FD_DEBUG, "%02x ", p[i]);
    }
    fprintf(VFS_FD_DEBUG, "\n");
}

void vga_set_cursor(int x, int y)
{
    int pos = y * screen_width + x;
    screen_x = x;
    screen_y = y;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_get_cursor(int *x, int *y)
{
    *x = screen_x;
    *y = screen_y;
}

// void vga_scrollback(int lines)
// {
//     for (int y = lines; y < screen_height; y++)
//     {
//         for (int x = 0; x < screen_width; x++)
//         {
//             uint32_t fg_color;
//             uint32_t bg_color;
//             char c = vga_get_cell(x, y, &fg_color, &bg_color);
//             video_text_set_cell(video_driver, x, y - lines, c, fg_color, bg_color);
//         }
//     }
//     for (int y = screen_height - lines; y < screen_height; y++)
//     {
//         for (int x = 0; x < screen_width; x++)
//         {
//             video_text_set_cell(video_driver, x, y - lines, ' ', default_color, 0);
//         }
//     }

//     screen_y -= lines;
// }

void vga_put_char(char c)
{
    switch (c)
    {
        case '\n' :
            screen_x = 0;
            screen_y++;
            break;

        case '\t' :
            for (int i = 0; i < 4 - (screen_x % 4); i++)
            {
                vga_put_char(' ');
            }
            break;

        case '\r' :
            screen_x = 0;
            break;

        default :
            // uint32_t color;
            video_text_set_cell(video_driver, screen_x, screen_y, c, default_color, 0);
            screen_x++;
            break;
    }

    if (screen_x >= 80)
    {
        screen_y++;
        screen_x = 0;
    }

    vga_set_cursor(screen_x, screen_y);
}

void vga_set_mode(framebuffer_t *mode)
{
    screen_width = mode->width;
    screen_height = mode->height;
}

void vga_write_char(device_t *dev, char c)
{
    vga_put_char(c);
}

size_t vga_write(void *buf, off_t offset, size_t count, device_t *dev)
{
    uint32_t offset_x = 0;
    uint32_t offset_y = 0;
    fb_unpack_index(&fb, offset, &offset_x, &offset_y);
    screen_x += offset_x;
    screen_y += offset_y;
    char *buffer = (char *)buf;
    for (size_t i = 0; i < count; i++)
    {
        vga_put_char(buffer[i]);
    }
    return count;
}

int vga_ioctl(uint32_t cmd, void *arg, device_t *dev)
{
    ENTER_FUNC(MODULE, "0x%x, %p, %p", cmd, arg, dev);
    switch (cmd)
    {
        case VIDEO_IOCTL_CLEAR :
            device_ioctl(video_driver, VIDEO_IOCTL_CLEAR, NULL);
            vga_set_cursor(0, 0);
            return 0;
        default :
            return -EINVAL;
    }
}


void vga_get_ops_dev(device_t *dev, tty_dev_ops_t *out)
{
    out->write_char = vga_write_char;
}

int vga_init(video_backend_t *backend)
{
    video_driver = device_get_by_id(DEVICE_VIDEO, 0);
    device_ioctl(video_driver, VIDEO_IOCTL_GET_INFO, &fb);
    default_color = video_pack_color(&fb, 0xFF, 0xFF, 0xFF);

    device_t *vga_dev = malloc(sizeof(device_t));
    vga_dev->class_name = "VGA";
    vga_dev->type = DEVICE_VIDEO;
    vga_dev->flags = DEVICE_FLAG_RW | DEVICE_FLAG_BLOCKDEV;
    vga_dev->ioctl = vga_ioctl;
    vga_dev->write = vga_write;
    vga_dev->tty_ops = vga_get_ops_dev;

    device_register(vga_dev);
    return RETURN_FAILED;
}

static video_ops_t vga_ops = {
    .init = vga_init,
    .set_pixel = vga_set_pixel,
    .fill_rect = vga_fill_rect,
    .flip = NULL, // no double buffer yet
    .shutdown = vga_shutdown,
};

video_ops_t *vga_get_ops()
{
    vga_ops.init = vga_init;
    vga_ops.set_pixel = vga_set_pixel;
    vga_ops.fill_rect = vga_fill_rect;
    vga_ops.shutdown = vga_shutdown;
    return &vga_ops;
}

/*
 * File: video.c
 * File Created: 27 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
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
#include "VGATextDevice.h"

#include "libs/stdio.h"
#include "libs/memory.h"

uint8_t* frame_buffer;
int current_mode;
video_mode *video_current_mode_data;
video_mode** modes;
int mode_count;


// Helper Functions
video_mode* video_get_mode_data(int mode)
{
    for (size_t i = 0; i < mode_count; i++)
    {
        video_mode* element = modes[i];
        // log_debug("VIDEO", "video mode %d==%d", element->mode, mode);
        if (element->mode == mode)
        {
            // log_debug("VIDEO", "found video mode %d %dx%dx%d fb=%p mode&=%p", i, element->width, element->height, element->bpp, element->frame_buffer, element);
            return modes[i];
        }
    }
    return NULL;
}

void video_set_pixel(uint32_t cursor_x, uint32_t cursor_y, uint32_t color_packed)
{
    int bytes_per_pixel = video_current_mode_data->bpp / 8;
    // TODO make draw functions for the others so not 24 bpp
    int index = (cursor_y * video_current_mode_data->width + cursor_x) * 3;
    frame_buffer = (uint8_t*)(uint64_t)video_current_mode_data->frame_buffer;
    // log_debug("VIDEO", "index = %X (%u * %u + %u) + %X", index, cursor_y, video_current_mode_data->width, cursor_x, frame_buffer);

    // log_debug("VIDEO", "red n & %x (%u) ? %x (%u)", video_mode.red_mask_size, video_mode.red_mask_size, video_mode.red_field_position, video_mode.red_field_position);
    // log_debug("VIDEO", "green n & %x (%u) ? %x (%u)", video_mode.greenMaskSize, video_mode.greenMaskSize, video_mode.greenFieldPosition, video_mode.greenFieldPosition);
    // log_debug("VIDEO", "blue n & %x (%u) ? %x (%u)", video_mode.blueMaskSize, video_mode.blueMaskSize, video_mode.blueFieldPosition, video_mode.blueFieldPosition);
    // log_debug("VIDEO", "alpha n & %x (%u) ? %x (%u)", video_mode.alphaMaskSize, video_mode.alphaMaskSize, video_mode.alphaFieldPosition, video_mode.alphaFieldPosition);

    uint8_t red = color_packed >> 16 & 0xFF;
    uint8_t green = color_packed >> 8 & 0xFF;
    uint8_t blue = color_packed & 0xFF;

    // 0x00RRGGBB
/* 
log_debug("VIDEO", "red %X & %x", red, video_mode.red_mask_size);
log_debug("VIDEO", "green %X & %x", green, video_mode.greenMaskSize);
log_debug("VIDEO", "blue %X & %x", blue, video_mode.blueMaskSize); */

    // uint8_t alpha = color_packed >> 24 & 0xFF;
    // function draw_24_bpp
    frame_buffer[index] = red;
    frame_buffer[index + 1] = green;
    frame_buffer[index + 2] = blue;
    // log_debug("VIDEO", "put the pixel");
    // frame_buffer[index] = alpha;

}

void video_set_mode(int mode)
{
    video_current_mode_data = video_get_mode_data(mode);
    vga_set_mode(video_current_mode_data);

    frame_buffer = (uint8_t*)(uint64_t)video_current_mode_data->frame_buffer;
    current_mode = mode;
}

void video_init(boot_params *bp, void *mode_addr)
{
    modes = (video_mode**)mode_addr;
    mode_count = bp->vesaModeCount;
    VESA_bios_info vesa_info = bp->vesaBios;
    VESA_mode* vesa_modes = bp->vesaModes;

    for (size_t i = 0; i < mode_count; i++)
    {
        modes[i] = (video_mode*)(mode_addr + sizeof(video_mode) * i);
        modes[i]->mode_attributes = vesa_modes[i].mode_attributes;
        modes[i]->memory_model = vesa_modes[i].memory_model;
        modes[i]->width = vesa_modes[i].width;
        modes[i]->height = vesa_modes[i].height;
        modes[i]->bpp = vesa_modes[i].bpp;
        modes[i]->frame_buffer = vesa_modes[i].frame_buffer;
        modes[i]->pitch = vesa_modes[i].pitch;
        modes[i]->red_mask_size = vesa_modes[i].red_mask_size;
        modes[i]->red_field_position = vesa_modes[i].red_field_position;
        modes[i]->greenMaskSize = vesa_modes[i].greenMaskSize;
        modes[i]->greenFieldPosition = vesa_modes[i].greenFieldPosition;
        modes[i]->blueMaskSize = vesa_modes[i].blueMaskSize;
        modes[i]->blueFieldPosition = vesa_modes[i].blueFieldPosition;
        modes[i]->alphaMaskSize = vesa_modes[i].alphaMaskSize;
        modes[i]->alphaFieldPosition = vesa_modes[i].alphaFieldPosition;
        modes[i]->mode = vesa_modes[i].mode;
        // memcpy(modes[i] + 3, (&vesa_modes[i]) + 4, 23);
    }

    video_set_mode(bp->currentMode);
}
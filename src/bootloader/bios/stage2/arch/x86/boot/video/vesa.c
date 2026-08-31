/*
 * File: vesa.c
 * File Created: 29 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "video_detect.h"
#include "memory.h"
#include "bios/bios.h"
#include "stdio.h"
#include "x86.h"

#include <defs.h>

typedef struct vbe_info_block
{
    char vbe_signature[4];      // == "VESA"
    uint16_t vbe_version;       // == 0x0300 for VBE 3.0
    uint16_t oem_string_ptr[2]; // isa vbeFarPtr
    uint32_t capabilities;
    uint16_t video_mode_ptr[2]; // isa vbeFarPtr
    uint16_t total_memory;      // as # of 64KB blocks
    uint8_t Reserved[492];
} PACKED vbe_info_block_t;

typedef struct vesa_mode_info
{
    uint16_t attributes;  // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;     // deprecated
    uint8_t window_b;     // deprecated
    uint16_t granularity; // deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;        // number of bytes per horizontal line
    uint16_t width;        // width in pixels
    uint16_t height;       // height in pixels
    uint8_t w_char;        // unused...
    uint8_t y_char;        // ...
    uint8_t planes;
    uint8_t bpp;           // bits per pixel in this mode
    uint8_t banks;         // deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size;     // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t frame_buffer;        // physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size; // size of memory in the frame_buffer but not being displayed on the screen
    uint8_t reserved1[206];
} PACKED vesa_mode_info_t;

void mm_arch_detect_video(boot_params_t *bt)
{
    vbe_info_block_t block;
    bios_regs_t out_regs;

    bios_arch_init_regs(&out_regs);
    bios_regs_t regs;
    bios_arch_init_regs(&regs);
    regs.ax = 0x4F00;
    seg_off_t memory = mem_linear_to_segoff(&block);
    regs.es = memory.segment;
    regs.di = memory.offset;
    regs.flags = 0;
    memset(&block, 0, sizeof(vbe_info_block_t));
    bios_arch_intcall(0x15, &regs, &out_regs);
    bios_arch_dump_frame(&out_regs);
    if (out_regs.ah != 0x4F)
    {
        printf("VESA is some how not supported... HOW? Got code 0x%x", out_regs.al);
        return;
    }
    
    bios_arch_init_regs(&out_regs);
    vesa_mode_info_t mode;
    memset(&mode, 0, sizeof(vesa_mode_info_t));
    bios_arch_init_regs(&regs);
    regs.ax = 0x4F01;
    memory = mem_linear_to_segoff(&mode);
    regs.es = memory.segment;
    regs.di = memory.offset;
    regs.cx = 0x2000 | 0x115;
    regs.flags = 0;
    bios_arch_intcall(0x15, &regs, &out_regs);
    if (out_regs.ah != 0x4F)
    {
        printf("VESA is some how not supported... HOW? Got code 0x%x", out_regs.al);
        return;
    }
    bt->video.count = 1;
    bt->video.entries[0].mode = 0x115;
    bt->video.entries[0].addr = mode.frame_buffer;
    bt->video.entries[0].width = mode.width;
    bt->video.entries[0].height = mode.height;
    bt->video.entries[0].pitch = mode.pitch;
    bt->video.entries[0].bpp = mode.bpp;
    bt->video.entries[0].red_shift = mode.red_position;
    bt->video.entries[0].green_shift = mode.green_position;
    bt->video.entries[0].blue_shift = mode.blue_position;
    bt->video.entries[0].fmt = FB_PIXEL_RGB;
}

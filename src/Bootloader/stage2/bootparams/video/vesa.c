/*
 * File: vesa.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vesa.h"
#include "x86.h"

#include "stdio.h"
#include "memdefs.h"
#include "minmax.h"

#include <stdbool.h>

uint16_t VESAModes[] = {
    0x115,
    0xFFFF,
};

#define MAX_REGIONS MAX_VIDEO_MODES

void hexdump(void *ptr, int len);

void DetectVESA(boot_params_t *bp)
{
    VbeInfoBlock local_info;
    bool vesa_supported = x86_VESASupported(&local_info);

    int vesa_entries_count;
    if (vesa_supported)
    {
        uint32_t index = 0;
        int ret = 1;

        vesa_entries_count = 0;

        vesa_mode_info_t pmode;
        while (ret > 0)
        {
            uint16_t moderaw = VESAModes[index];
            if (moderaw == 0xFFFF)
            {
                break;
            }
            ret = x86_GetVESAEntry(0x2000 | (moderaw & 0x1FF), &pmode);
            printf("vesa mode %d %dx%dx%d ret=%u pmode=0x%x\n", moderaw,
                   pmode.width, pmode.height, pmode.bpp, ret, pmode);
            bp->video.entries[index].mode = moderaw;
            bp->video.entries[index].width = pmode.width;
            bp->video.entries[index].height = pmode.height;
            bp->video.entries[index].bpp = pmode.bpp;
            bp->video.entries[index].addr = (uint64_t)pmode.frame_buffer;
            bp->video.entries[index].pitch = pmode.pitch;
            bp->video.entries[index].red_shift = pmode.red_position;
            bp->video.entries[index].green_shift = pmode.green_position;
            bp->video.entries[index].blue_shift = pmode.blue_position;
            bp->video.entries[index].fmt = FB_PIXEL_BITMASK;
            vesa_entries_count++;
            index++;
        }
    }

    bp->video.count = vesa_entries_count;
    printf("VESA: got %u entries\n", vesa_entries_count);
}

void SetVESAMode(int mode)
{
    // bool isGraphic = false;
    if (mode == -1)
    {
        printf("Mode is %u\n", mode);
        return;
    }
    vesa_mode_info_t vesaModeData;
    int ret = x86_GetVESAEntry((mode & 0x1FF) | 0x4000, &vesaModeData);
    if (ret == 0)
    {
        printf("ret is %u == 0\n", ret);
        return;
    }
    printf("ret = %u attributes = 0x%x\n", ret, vesaModeData.attributes);

    int vesaMode = mode | 0x4000;

    ret = x86_SetVESAMode(vesaMode);
    printf("result = %x\n", ret);
    if (ret != 0x004f)
    {
        printf("Didnt work can't set mode for vesa\n");
        // SetVGAMode(0);
    }
}

/*
 * File: vesa.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
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
    0x00,
    0x02,
    0x04,
    0x06,
    0x07,
    0x0D,
    0x0E,
    0x0F,
    0x10,
    0x11,
    0x12,
    0x13,
    0x100,
    0x101,
    0x102,
    0x103,
    0x105,
    0x107,
    0x110,
    0x111,
    0x112,
    0x113,
    0x114,
    0x115,
    0x116,
    0x117,
    0xFFFF,
};

#define MAX_REGIONS MAX_VESA_MODES

bool VESASupported;
vesa_mode_info_t g_VesaEntries[MAX_REGIONS];
VbeInfoBlock *Info;

void hexdump(void *ptr, int len);

void DetectVESA(BootParams *bp)
{
    int g_VesaEntriesCount;
    Info = (VbeInfoBlock *)(void *)&bp->vesaBios;
    VESASupported = x86_VESASupported(Info);
    bp->vesaBios.capabilities = Info->Capabilities;
    bp->vesaBios.totalMemory = Info->TotalMemory;
    bp->vesaBios.vbeVersion = Info->VbeVersion;

    uint32_t phys = (Info->VideoModePtr[1] << 4) + Info->VideoModePtr[0];
    uint16_t *src = (uint16_t *)phys;
    int count = 0;
    while (*src != 0xFFFF && count < MAX_VESA_MODES)
    {
        bp->vesaBios.modeList[count++] = *src;
        src++;
    }
    if (*src != 0xFFFF)
    {
        printf("Vesa has more modes");
    }

    if (VESASupported)
    {
        uint32_t index = 0;
        int ret = 1;

        g_VesaEntriesCount = 0;

        vesa_mode_info_t pmode;
        while (ret > 0)
        {
            uint16_t moderaw = VESAModes[index];
            if (moderaw == 0xFFFF)
            {
                break;
            }
            ret = x86_GetVESAEntry(0x2000 | (moderaw & 0x1FF), &pmode);
            printf("vesa mode %d %dx%dx%d ret=%u pmode=0x%x\n", moderaw, pmode.width, pmode.height, pmode.bpp, ret, pmode);
            bp->vesaModes[index].modeAttributes = pmode.attributes;
            bp->vesaModes[index].memoryModel = pmode.memory_model;
            bp->vesaModes[index].mode = moderaw;
            bp->vesaModes[index].width = pmode.width;
            bp->vesaModes[index].height = pmode.height;
            bp->vesaModes[index].bitsPerPixel = pmode.bpp;
            bp->vesaModes[index].framebuffer = pmode.framebuffer;
            bp->vesaModes[index].pitch = pmode.pitch;
            bp->vesaModes[index].redMaskSize = pmode.red_mask;
            bp->vesaModes[index].redFieldPosition = pmode.red_position;
            bp->vesaModes[index].greenMaskSize = pmode.green_mask;
            bp->vesaModes[index].greenFieldPosition = pmode.green_position;
            bp->vesaModes[index].blueMaskSize = pmode.blue_mask;
            bp->vesaModes[index].blueFieldPosition = pmode.blue_position;
            bp->vesaModes[index].alphaMaskSize = pmode.reserved_mask;
            bp->vesaModes[index].alphaFieldPosition = pmode.reserved_position;
            g_VesaEntriesCount++;
            index++;
        }
    }

    bp->vesaModeCount = g_VesaEntriesCount;
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
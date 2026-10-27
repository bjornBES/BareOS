/*
 * File: bochs.c
 * File Created: 20 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/driver.h"

#include "drivers/pci/pci.h"

#include "drivers/video/display.h"

#include "resource/resource.h"

#include "mm/ioremap.h"

#include "kerrno.h"

#include "memory.h"
#include "string.h"

#include <types.h>

#define VBE_DISPI_IOPORT_INDEX           0x01CE
#define VBE_DISPI_IOPORT_DATA            0x01CF

#define VBE_DISPI_INDEX_ID               0x0
#define VBE_DISPI_INDEX_XRES             0x1
#define VBE_DISPI_INDEX_YRES             0x2
#define VBE_DISPI_INDEX_BPP              0x3
#define VBE_DISPI_INDEX_ENABLE           0x4
#define VBE_DISPI_INDEX_BANK             0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH       0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT      0x7
#define VBE_DISPI_INDEX_X_OFFSET         0x8
#define VBE_DISPI_INDEX_Y_OFFSET         0x9
#define VBE_DISPI_INDEX_VIDEO_MEMORY_64K 0xA

#define VBE_DISPI_ID0                    0xB0C0
#define VBE_DISPI_ID1                    0xB0C1
#define VBE_DISPI_ID2                    0xB0C2
#define VBE_DISPI_ID3                    0xB0C3
#define VBE_DISPI_ID4                    0xB0C4
#define VBE_DISPI_ID5                    0xB0C5

#define DRV_NAME                         "bochs"

#define MODULE                           "bochs"

typedef struct bochs_device
{
    vaddr_t mmio;

    paddr_t fb_map;
    paddr_t fb_base;
    vaddr_t fb_size;
    unsigned long qext_size;

    /* mode */
    uint16_t xres;
    uint16_t yres;
    uint16_t yres_virtual;
    uint32_t stride;
    uint32_t bpp;
} bochs_device_t;

uint16_t bochs_dispi_read(bochs_device_t *bochs_dev, uint16_t reg)
{
    if (bochs_dev->mmio)
    {
        return *(volatile uint16_t *)((0x500) + (reg << 1) + bochs_dev->mmio);
    }

    return 0;
}

status_t bochs_dispi_write(bochs_device_t *bochs_dev, uint16_t reg, uint16_t val)
{
    if (bochs_dev->mmio)
    {
        *(volatile uint16_t *)((0x500) + (reg << 1) + bochs_dev->mmio) = val;
        return KERRNO_SUCCESSES;
    }

    return 0;
}

status_t bochs_set_mode(display_device_t *dev, const display_mode_t *mode)
{
    ENTER_FUNC("%p, %p", dev, mode);
    bochs_device_t *b = dev->priv;

    if ((uint64_t)mode->w * mode->h * (mode->bpp / 8) > b->fb_size)
    {
        KERRNO_RETURN(EINVAL, "");
    }

    bochs_dispi_write(b, VBE_DISPI_INDEX_ENABLE, 0);
    bochs_dispi_write(b, VBE_DISPI_INDEX_XRES, mode->w);
    bochs_dispi_write(b, VBE_DISPI_INDEX_YRES, mode->h);
    bochs_dispi_write(b, VBE_DISPI_INDEX_BPP, mode->bpp);
    bochs_dispi_write(b, VBE_DISPI_INDEX_ENABLE, 0x01 | 0x40);

    dev->frame_buffer.width = mode->w;
    dev->frame_buffer.height = mode->h;
    dev->frame_buffer.bpp = mode->bpp;
    dev->frame_buffer.pitch = bochs_dispi_read(b, VBE_DISPI_INDEX_VIRT_WIDTH) * mode->bpp / 8;
    return KERRNO_SUCCESSES;
}

static display_driver_t bochs_video_device = {
    .name = DRV_NAME,
    .set_mode = bochs_set_mode,
};
// MODULE_VIDEO_DRIVER(bochs_video_device)

status_t bochs_pci_probe(pci_device_t *pdev, pci_device_id_t *ent)
{
    ENTER_FUNC("%p, %p", pdev, ent);

    status_t ret = 0;

    ret = pci_set_bus_master(pdev, 1);
    ret |= pci_set_command_flags(pdev, PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
    ret |= pci_clear_command_flags(pdev, PCI_COMMAND_INTX_DISABLE);
    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }

    resource_t *framebuffer = resource_create();

    pci_bar_t memory_bar;
    pci_bar_t io_bar;
    ret = pci_get_bar(pdev, 0, &memory_bar);
    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }

    trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", memory_bar.base, memory_bar.is_64bit BOOL_TO_STRING, memory_bar.is_mmio BOOL_TO_STRING, memory_bar.prefetchable BOOL_TO_STRING, memory_bar.size);
    resource_request(&mem_space.root, framebuffer, memory_bar.base, memory_bar.base + memory_bar.size, "framebuffer", RES_TYPE_PCI_BAR, RES_FLAG_NONE);

    resource_t *bochs_io = resource_create();
    resource_request(&io_space.root, bochs_io, 0x1CE, 0x1DE, "bochs vbe io", 0, RES_FLAG_NONE);

    bochs_device_t *bochs_dev = kzmalloc(sizeof(bochs_device_t));
    bochs_dev->fb_base = framebuffer->start;
    bochs_dev->fb_size = memory_bar.size;
    bochs_dev->fb_map = ioremap(framebuffer->start, memory_bar.size);

    display_device_t *display_dev = display_create();
    display_dev->driver = &bochs_video_device;
    display_dev->priority = 5;

    ret = pci_get_bar(pdev, 2, &io_bar);
    if (ret == KERRNO_SUCCESSES)
    {
        trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", io_bar.base, io_bar.is_64bit BOOL_TO_STRING, io_bar.is_mmio BOOL_TO_STRING, io_bar.prefetchable BOOL_TO_STRING, io_bar.size);
        resource_request(&mem_space.root, &pdev->driver_exclusive_resource, io_bar.base, io_bar.base + io_bar.size, "mmio", RES_TYPE_PCI_BAR, RES_FLAG_NONE);
        bochs_dev->mmio = ioremap(io_bar.base, io_bar.size);
        resource_t *vga_io = resource_create();
        resource_request(&io_space.root, vga_io, 0x3C0, 0x3DF, "standard vga io", 0, RES_FLAG_NONE);
    }
    else
    {
        resource_request(&io_space.root, &pdev->driver_exclusive_resource, 0x3C0, 0x3DF, "standard vga io", 0, RES_FLAG_NONE);
    }

    uint16_t id = bochs_dispi_read(bochs_dev, VBE_DISPI_INDEX_ID);
    size_t mem = bochs_dispi_read(bochs_dev, VBE_DISPI_INDEX_VIDEO_MEMORY_64K) * 64 * 1024;
    if ((id & 0xFFF0) != VBE_DISPI_ID0)
    {
        KERRNO_RETURN(ENODEV, "");
    }

    if (mem != memory_bar.size)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bochs size is not the same as bar size");
    }

    display_dev->frame_buffer.addr = bochs_dev->fb_map;
    display_dev->frame_buffer.phys_addr = framebuffer->start;
    display_dev->frame_buffer.bpp = 32;
    display_dev->frame_buffer.size = mem;
    display_dev->frame_buffer.b_pos = 0;
    display_dev->frame_buffer.g_pos = 8;
    display_dev->frame_buffer.r_pos = 16;

    display_dev->dev.init_name = "bochs";

    trace_info(MODULE, "mem = %p, id = 0x%x", mem, id);
    trace_info(MODULE, "mem = %p, id = 0x%x", mem, id);

    display_dev->priv = bochs_dev;

    display_register(display_dev);

    /*     while (true)
        {
            ;
        } */

    return KERRNO_SUCCESSES;
}

void bochs_pci_remove(pci_device_t *pdev)
{
}

static pci_device_id_t bochs_pci_tbl[] = {
    {
        .device = 0x1111,
        .vendor = 0x1234,
        .class = PCI_CLASS_DISPLAY_VGA,
        .class_mask = 0xFFFF00,
        // .vendor = PCI_SUBVENDOR_ID_REDHAT_QUMRANET,
        // .device = PCI_SUBDEVICE_ID_QEMU,
        // .driver_data = BOCHS_QEMU_STDVGA,
    },
    {
        .device = 0x1111,
        .vendor = 0x1234,
        .class = 0,
        .class_mask = 0,
        // .vendor = PCI_ANY_ID,
        // .device = PCI_ANY_ID,
        // .driver_data = BOCHS_UNKNOWN,
    },
    {
        .device = 0x1111,
        .vendor = 0x4321,
        .class = 0,
        .class_mask = 0,
        // .vendor = PCI_ANY_ID,
        // .device = PCI_ANY_ID,
        // .driver_data = BOCHS_SIMICS,
    },
    {} /* terminate list */
};

static pci_driver_t bochs_pci_driver = {
    .name = DRV_NAME,
    .id_table = bochs_pci_tbl,
    .probe = bochs_pci_probe,
    .remove = bochs_pci_remove,
};

MODULE_PCI_DRIVER(bochs_pci_driver)

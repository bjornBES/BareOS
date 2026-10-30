/*
 * File: sata.c
 * File Created: 08 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "sata.h"
#include "ahci.h"

#include "drivers/base/block/blkdev.h"

#include "asm/mmu_arch.h"

#include "dev/device.h"

#include "sync/mutex.h"
#include "sync/semaphore.h"

#include "memory.h"

#include <types.h>

#define MODULE               "SATA"

#define SATA_READ_DMA_EX     0x25
#define SATA_WRITE_DMA_EX    0x35
#define SATA_IDENTIFY_DEVICE 0xEC
#define SATA_BUSY            0x80
#define SATA_DRQ             0x08

typedef struct
{
} sata_cmd_ctx_t;

typedef struct ahci_port
{
    sata_cmd_ctx_t cmds[32];
    hba_mem_t *abar;
    ahci_port_priv_t *ahci_port;
} sata_port_t;

uint32_t sata_find_cmdslot(ahci_port_priv_t *aport)
{
    ENTER_FUNC("%p", aport);
    hba_port_t *port = aport->port;
    uint32_t slots = (port->sact | port->ci);
    uint32_t cmdslots = aport->host_priv->abar->cap.NCS;
    for (uint32_t i = 0; i < cmdslots; i++)
    {
        if ((slots & 1) == 0)
        {
            return i;
        }
        slots >>= 1;
    }
    return 0xFFFFFFFF;
}

status_t sata_read_sectors(blkdev_t *blk_dev, void *buf, size_t count, off_t sector, ssize_t *size_out)
{
    // log_debug(MODULE, "ahci_identify_device(%p, %p)", aport, buf);
    // log_debug(MODULE, "buf = p%p / v%p", buf, buf_virt);
    ahci_port_priv_t *aport = blk_dev->dev.drv_data;

    ahci_cmd_request_t req = {
        .command = SATA_READ_DMA_EX,
        .lba = sector,
        .is_write = false,
        .buffer = (vaddr_t)buf,
        .sector_count = count,
        .buffer_size = (count * blk_dev->sector_size),
    };
    *size_out = count;
    return ahci_submit(aport, &req);
}

int sata_identify_aport(sata_port_t *aport, ahci_port_priv_t *ahci_port, vaddr_t buf_virt)
{
    ENTER_FUNC("%p, %p", aport, buf_virt);
    // log_debug(MODULE, "ahci_identify_device(%p, %p)", aport, buf);
    // log_debug(MODULE, "buf = p%p / v%p", buf, buf_virt);

    /*     ahci_cmd_request_t req = {
            .command = SATA_IDENTIFY_DEVICE, // IDENTIFY DEVICE
            .lba = 0,                        // not used by this command
            .sector_count = 0,               // not used by this command
            .is_write = false,               // it's a read device sends data to us
            .buffer = buf_virt,
            .buffer_size = 512,              // IDENTIFY always returns exactly one 512-byte sector
        };
        return ahci_submit(aport->ahci_port, &req); */

    hba_port_t *port = aport->ahci_port->port;
    port->is.is = 0xFFFFFFFF;
    uint32_t slot = sata_find_cmdslot(ahci_port);
    if (slot == 0xFFFFFFFF)
    {
        return 1;
    }

    ahci_cmd_header_t *cmd_header = (ahci_cmd_header_t *)ahci_port->clb;
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmd_header->w = 0;

    HBA_CMD_TBL *cmd_table = (HBA_CMD_TBL *)ahci_port->ctba[slot];

    cmd_table->prdt_entry[0].dba = (uint64_t)mmu_arch_virt_to_phys(&kernel_page, buf_virt);
    cmd_table->prdt_entry[0].dbc = 511;
    cmd_table->prdt_entry[0].i = 1;

    FIS_REG_H2D *cmd_fis = (FIS_REG_H2D *)(&cmd_table->cfis);

    cmd_fis->fis_type = FIS_TYPE_REG_H2D; // Host to device
    cmd_fis->c = 1;
    cmd_fis->command = SATA_IDENTIFY_DEVICE;

    cmd_fis->device = 1 << 6;

    for (uint32_t spin = 0; spin < 1000000; spin++)
    {
        if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
        {
            break;
        }
    }
    if (port->tfd & (SATA_BUSY | SATA_DRQ))
    {
        return 2;
    }

    port->ci = (1 << slot);

    while (1)
    {
        if (!(port->ci & (1 << slot)))
        {
            break;
        }
        if (port->is.is & (1 << 30))
        {
            return 3;
        }
    }

    if (port->is.is & (1 << 30))
    {
        return 3;
    }
    return KERRNO_SUCCESSES;
}

extern void hexdump(void *ptr, size_t len, size_t size);

status_t sata_init_port(pci_device_t *pci_dev, ahci_host_priv_t *host, ahci_port_priv_t *aport, blkdev_t **blk_dev_out)
{
    ENTER_FUNC("%p, %p, %p, %p", pci_dev, host, aport, blk_dev_out);
    sata_port_t *sata_aport = kzmalloc(sizeof(sata_port_t));
    sata_aport->abar = host->abar;
    sata_aport->ahci_port = aport;
    aport->port_priv = sata_aport;

    aport->identify = kzmalloc(sizeof(ata_identify_packet_t));

    ahci_initialize_aport(aport);
    sata_identify_aport(sata_aport, aport, (vaddr_t)aport->identify);

    blkdev_t *blk_dev;
    status_t ret = block_device_create(&blk_dev);
    KERRNO_CHECK_RETURN(ret);

    *blk_dev_out = blk_dev;
    
    blk_dev->ops.read_sectors = sata_read_sectors;
    
    aport->drive = host->ahci_devices_count;

    trace_debug(MODULE, "getting name");
    char *name = (char *)kzmalloc(41);
    for (int i = 0; i < 40; i += 2)
    {
        if (aport->identify->model[i] == ' ' && aport->identify->model[i + 1] == ' ')
        {
            break;
        }
        name[i] = aport->identify->model[i + 1];
        name[i + 1] = aport->identify->model[i];
    }

    blk_dev->dev.drv_data = aport;
    // dev->read = ahci_read;
    // dev->write = ahci_write;
    blk_dev->dev.init_name = "sata";
    // dev->flags = DEVICE_FLAG_RW | DEVICE_FLAG_BLOCKDEV;

    // partition_block_device_register(dev);

    uint64_t total_sectors = aport->identify->lba28_sectors;
    if (aport->identify->cmd_set_en1 & (1 << 10))
    {
        total_sectors = aport->identify->lba48_sectors;
    }

    // uint16_t sector_size = aport->identify->physical_logical & 0xF000;
    // trace_info(MODULE, "size = 0x%x, 0x%x", (sector_size >> (16-4)), aport->identify->logical_sector_size);

    if (aport->identify->physical_logical & BIT(3))
    {
        FUNC_NOT_IMPLEMENTED();
        blk_dev->sector_size = 0;
    }
    else
    {
        blk_dev->sector_size = 512;
    }

    blk_dev->sector_start = 0;
    blk_dev->sectors_count = total_sectors;

    // printf("Detected SATA drive: %s (%u MiB)\n", modelName, info.total_sectors / 2048);
    trace_info(MODULE, "Detected SATA drive: %s (%u MiB)", name, total_sectors / 2048);

    return KERRNO_SUCCESSES;
}

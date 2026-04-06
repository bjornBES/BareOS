/*
 * File: ahci.c
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ahci.h"
#include "debug/debug.h"
#include "partition_manager/partition_manager.h"
#include "device/device.h"
#include "arch/x86/paging/paging.h"

#include "libs/malloc.h"
#include "libs/stdio.h"
#include "libs/memory.h"

#include <stdbool.h>

#define MODULE "AHCI"

#define SATA_SIG_ATA 0x00000101   // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101    // Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define HBA_DET_PRESENT 3
#define HBA_IPM_ACTIVE 1
#define HBA_CMD_CR (1 << 15)
#define HBA_CMD_FR (1 << 14)
#define HBA_CMD_FRE (1 << 4)
#define HBA_CMD_SUD (1 << 1)
#define HBA_CMD_ST (1)

#define SATA_READ_DMA_EX 0x25
#define SATA_WRITE_DMA_EX 0x35
#define SATA_IDENTIFY_DEVICE 0xEC
#define SATA_BUSY 0x80
#define SATA_DRQ 0x08

#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000

#define MAX_PORTS 4

// 32 slots × 256 bytes each = 8KB total (2 pages)
// 256 gives room for CFIS(128) + 8 PRDTs(128) with alignment headroom
#define CTBA_SLOT_SIZE 256

typedef struct
{
    HBA_MEM *abar;
    HBA_PORT *port;
    void *clb;
    void *fb;
    void *ctba[32];
    void *unused[28]; // Even out the data size to 256 bytes
} ahci_port;

ahci_port *ports;
int ahci_devices_count;

uint32_t ahci_find_cmdslot(ahci_port aport)
{
    HBA_PORT *port = aport.port;
    uint32_t slots = (port->sact | port->ci);
    uint32_t cmdslots = (aport.abar->cap & 0x0f00) >> 8;
    for (uint32_t i = 0; i < cmdslots; i++)
    {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    return 0xFFFFFFFF;
}

uint8_t ahci_identify_device(ahci_port aport, void *buf)
{
    log_debug(MODULE, "ahci_identify_device(aport, p%p)", buf);
    log_debug(MODULE, "buf = p%p / v%p", buf, paging_get_virtual(kernel_page, buf));
    HBA_PORT *port = aport.port;
    port->is = 0xFFFFFFFF;
    uint32_t slot = ahci_find_cmdslot(aport);
    if (slot == 0xFFFFFFFF)
        return 1;

    HBA_CMD_HEADER *cmd_header = (HBA_CMD_HEADER *)aport.clb;
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmd_header->w = 0;

    HBA_CMD_TBL *cmd_table = (HBA_CMD_TBL *)aport.ctba[slot];

    cmd_table->prdt_entry[0].dba = (uint64_t)paging_get_physical(kernel_page, buf);
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
            break;
    }
    if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
        return 2;

    port->ci = (1 << slot);

    log_debug(MODULE, "issuing command: tfd=%x cmd=%x ci=%x is=%x", port->tfd, port->cmd, port->ci, port->is);

    while (1)
    {
        if (!(port->ci & (1 << slot)))
            break;
        if (port->is & (1 << 30))
            return 3;
    }

    if (port->is & (1 << 30))
        return 3;

    return 0;
}

void ahci_start_cmd(HBA_PORT *port)
{
    while (port->cmd & HBA_PxCMD_CR)
        ;

    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

void ahci_stop_cmd(HBA_PORT *port)
{
    port->cmd &= ~HBA_PxCMD_ST;

    port->cmd &= ~HBA_PxCMD_FRE;

    while ((port->cmd & HBA_CMD_FR) || (port->cmd & HBA_CMD_CR))
        ;
}

int ahci_check_type(HBA_PORT *port)
{
    uint32_t ssts = port->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
        return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
        return AHCI_DEV_PM;
    default:
        return AHCI_DEV_SATA;
    }
}

void ahci_initialize_port(ahci_port *aport)
{
    HBA_PORT *port = aport->port;
    ahci_stop_cmd(port);

    void *mapped_clb_virt;
    void *mapped_clb = kmalloc_phys(4096, &mapped_clb_virt);
    port->clb = (uint64_t)mapped_clb;
    aport->clb = mapped_clb_virt;

    void *mapped_fb_virt;
    void *mapped_fb = kmalloc_phys(4096, &mapped_fb_virt);
    port->fb = (uint64_t)mapped_fb;
    aport->fb = mapped_fb_virt;

    HBA_CMD_HEADER *cmd_header = (HBA_CMD_HEADER *)mapped_clb;

    void *ctba_pool_virt;
    void *ctba_pool = kcalloc_phys(1, 32 * CTBA_SLOT_SIZE, &ctba_pool_virt);

    for (uint8_t i = 0; i < 32; i++)
    {
        void *ctba_buf = (void *)((uintptr_t)ctba_pool + i * CTBA_SLOT_SIZE);
        void *ctba_buf_virt = (void *)((uintptr_t)ctba_pool_virt + i * CTBA_SLOT_SIZE);
        cmd_header[i].prdtl = 1;
        aport->ctba[i] = ctba_buf_virt;
        cmd_header[i].ctba = (uint64_t)ctba_buf;
    }

    ahci_start_cmd(port);
}

bool ahci_read_sectors_command(ahci_port aport, uint32_t startl, uint32_t starth, size_t count, uint16_t *buf)
{
    HBA_PORT *port = aport.port;
    port->is = (uint32_t)-1; // Clear pending interrupt bits
    uint32_t slot = ahci_find_cmdslot(aport);

    if (slot == -1)
        return false;

    HBA_CMD_HEADER *cmd_header = (HBA_CMD_HEADER *)aport.clb;
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
    cmd_header->w = 0;                                        // Read from device

    HBA_CMD_TBL *cmd_tbl = (HBA_CMD_TBL *)(aport.ctba[slot]);

    cmd_tbl->prdt_entry[0].dba = (uint64_t)buf;
    cmd_tbl->prdt_entry[0].dbc = (count * 512) - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
    cmd_tbl->prdt_entry[0].i = 1;

    // Setup command
    FIS_REG_H2D *cmd_fis = (FIS_REG_H2D *)(&cmd_tbl->cfis);

    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1; // Command
    cmd_fis->command = SATA_READ_DMA_EX;

    cmd_fis->lba0 = (uint8_t)startl;
    cmd_fis->lba1 = (uint8_t)(startl >> 8);
    cmd_fis->lba2 = (uint8_t)(startl >> 16);
    cmd_fis->device = 1 << 6;

    cmd_fis->lba3 = (uint8_t)(startl >> 24);
    cmd_fis->lba4 = (uint8_t)(starth);
    cmd_fis->lba5 = (uint8_t)(starth >> 8);

    cmd_fis->countl = (count & 0xFF);
    cmd_fis->counth = (count >> 8);

    for (uint32_t spin = 0; spin < 1000000; spin++)
    {
        if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
        {
            break;
        }
    }
    if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
    {
        return 2;
    }

    port->ci = 1 << slot; // Issue command

    while (1)
    {
        if (!(port->ci & (1 << slot)))
            break;
        if (port->is & (1 << 30))
            return 3;
    }

    if (port->is & (1 << 30))
        return 3;

    return true;
}

bool ahci_write_sectors_command(ahci_port aport, uint32_t startl, uint32_t starth, size_t count, uint16_t *buf)
{
    HBA_PORT *port = aport.port;
    port->is = (uint32_t)-1; // Clear pending interrupt bits
    uint32_t slot = ahci_find_cmdslot(aport);

    if (slot == -1)
        return false;

    HBA_CMD_HEADER *cmd_header = (HBA_CMD_HEADER *)aport.clb;
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
    cmd_header->w = 1;                                        // Write to device

    HBA_CMD_TBL *cmd_tbl = (HBA_CMD_TBL *)(aport.ctba[slot]);

    cmd_tbl->prdt_entry[0].dba = (uint64_t)buf;
    cmd_tbl->prdt_entry[0].dbc = (count * 512) - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
    cmd_tbl->prdt_entry[0].i = 1;

    // Setup command
    FIS_REG_H2D *cmd_fis = (FIS_REG_H2D *)(&cmd_tbl->cfis);

    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1; // Command
    cmd_fis->command = SATA_WRITE_DMA_EX;

    cmd_fis->lba0 = (uint8_t)startl;
    cmd_fis->lba1 = (uint8_t)(startl >> 8);
    cmd_fis->lba2 = (uint8_t)(startl >> 16);
    cmd_fis->device = 1 << 6;

    cmd_fis->lba3 = (uint8_t)(startl >> 24);
    cmd_fis->lba4 = (uint8_t)(starth);
    cmd_fis->lba5 = (uint8_t)(starth >> 8);

    cmd_fis->countl = (count & 0xFF);
    cmd_fis->counth = (count >> 8);

    for (uint32_t spin = 0; spin < 1000000; spin++)
    {
        if (!(port->tfd & (SATA_BUSY | SATA_DRQ)))
        {
            break;
        }
    }
    if ((port->tfd & (SATA_BUSY | SATA_DRQ)))
    {
        return 2;
    }

    port->ci = 1 << slot; // Issue command

    while (1)
    {
        if (!(port->ci & (1 << slot)))
            break;
        if (port->is & (1 << 30))
            return 3;
    }

    if (port->is & (1 << 30))
        return 3;

    return true;
}

uint32_t ahci_read(void *buffer, uint64_t sector, size_t count, device *device)
{
    sata_private_data *priv = (sata_private_data *)device->priv;
    uint16_t drive = priv->drive;
    ahci_port aport = ports[drive];
    if (aport.abar != 0)
    {
        uint16_t *u16Buffer = (uint16_t *)paging_get_physical(kernel_page, buffer);
        uint32_t startl = sector & 0xFFFFFFFF;
        uint32_t starth = (sector >> 32) & 0xFFFFFFFF;
        if (ahci_read_sectors_command(aport, startl, starth, count, u16Buffer))
        {
            return count;
        }
    }
    else
    {
        return -1;
    }
    return -1;
}

uint32_t ahci_write(void *buffer, uint64_t sector, size_t count, device *device)
{
    sata_private_data *priv = (sata_private_data *)device->priv;
    uint16_t drive = priv->drive;
    ahci_port aport = ports[drive];
    if (aport.abar != 0)
    {
        uint16_t *u16Buffer = (uint16_t *)paging_get_physical(kernel_page, buffer);
        uint32_t startl = sector & 0xFFFFFFFF;
        uint32_t starth = (sector >> 32) & 0xFFFFFFFF;
        if (ahci_read_sectors_command(aport, startl, starth, count, u16Buffer))
        {
            return count;
        }
    }
    else
    {
        return -1;
    }
    return -1;
}

void ahci_initialize_abar(HBA_MEM *abar)
{
    uint32_t pi = abar->pi;
    for (size_t i = 0; i < 32; i++)
    {
        if (pi & 1)
        {
            // HBA_PORT = 128
            // HBA_MEM = 4352 (0x1100)

            int dt = ahci_check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {

                sata_identify_packet *info_virt;
                sata_identify_packet *info = kmalloc_phys(sizeof(sata_identify_packet), (void **)&info_virt);
                log_debug(MODULE, "info_virt = %p, info = %p", info_virt, info);

                mmPrintStatus();
                mmPrintBlocks();

                HBA_PORT *port = &abar->ports[i];
                log_debug(MODULE, "found sata");
                ports[ahci_devices_count].abar = abar;
                ports[ahci_devices_count].port = port;
                ahci_port *aport = &ports[ahci_devices_count];
                ahci_initialize_port(aport);
                log_debug(MODULE, "Port %u initialized", ahci_devices_count);
                ahci_identify_device(*aport, info);
                log_debug(MODULE, "Port %u identified", ahci_devices_count);

                device *dev = (device *)malloc(sizeof(device));
                sata_private_data *priv = (sata_private_data *)malloc(sizeof(sata_private_data));
                priv->drive = ahci_devices_count;

                dev->priv = priv;
                dev->device_id = 1;
                dev->type = DEVICE_DISK;
                dev->read = ahci_read;
                dev->write = ahci_write;
                device_add(dev);

                char *name = (char *)malloc(41);
                memset(name, 0, 41);
                // for (int i = 0; i < 40; i += 2)
                // {
                //     name[i] = info_virt->model_number[i + 1];
                //     name[i + 1] = info_virt->model_number[i];
                // }
                int count = sprintf(name, "sata%u", ahci_devices_count);
                name[count] = '\0';
                dev->name = name;

                partition_block_device_register(dev);

                // printf("Detected SATA drive: %s (%u MiB)\n", modelName, info.total_sectors / 2048);
                log_info(MODULE, "Detected SATA drive: %s (%u MiB)", name, info->total_sectors / 2048);
                ahci_devices_count++;

                free(info);
            }
        }
    }
}

void ahci_initialize(pci_device_id *pdev)
{
    ports = (ahci_port *)malloc(sizeof(ahci_port) * MAX_PORTS);

    phys_addr bar5 = (phys_addr)(uint64_t)pdev->header.header0.bar5;
    paging_map_region(kernel_page, bar5, bar5, 4096, -1);
    // log_warn("MAIN", "=========== nr.2 ===========");
    // frame_dump_bitmap();
    paging_map_region(kernel_page, bar5 + sizeof(HBA_MEM), bar5 + sizeof(HBA_MEM), 1, -1);
    // log_debug(MODULE, "got map p0x%x at v0x%x", bar5, paging_get_virtual(kernel_page, bar5));
    ahci_initialize_abar((HBA_MEM *)bar5);
    log_info(MODULE, "exit out");
}
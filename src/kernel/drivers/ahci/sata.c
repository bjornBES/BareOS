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
    uint16_t general_config;         // 0:  General configuration
    uint16_t num_cylinders;          // 1:  Number of cylinders (obsolete)
    uint16_t specific_config;        // 2:  Specific configuration
    uint16_t num_heads;              // 3:  Number of heads (obsolete)
    uint16_t rsv0[2];                // 4-5: Retired
    uint16_t num_sectors_per_track;  // 6:  Sectors per track (obsolete)
    uint16_t vendor0[3];             // 7-9: Vendor specific
    char serial[20];                 // 10-19: Serial number (ASCII, space-padded)
    uint16_t rsv1[2];                // 20-21: Retired
    uint16_t rsv2;                   // 22: Obsolete
    char firmware[8];                // 23-26: Firmware revision (ASCII, space-padded)
    char model[40];                  // 27-46: Model number (ASCII, space-padded)
    uint16_t max_sectors_per_drq;    // 47: Max sectors per DRQ (bits 7:0)
    uint16_t rsv3;                   // 48: Trusted computing / reserved
    uint16_t capabilities0;          // 49: Capabilities (DMA, LBA support)
    uint16_t capabilities1;          // 50: Capabilities extended
    uint16_t rsv4[2];                // 51-52: Obsolete
    uint16_t field_validity;         // 53: Field validity flags
    uint16_t rsv5[5];                // 54-58: Obsolete
    uint16_t multi_sector_setting;   // 59: Multiple sector setting
    uint32_t lba28_sectors;          // 60-61: Total LBA28 addressable sectors
    uint16_t rsv6;                   // 62: Obsolete
    uint16_t dma_multiword;          // 63: Multiword DMA modes
    uint16_t pio_modes;              // 64: PIO modes supported
    uint16_t min_dma_cycle_time;     // 65: Min multiword DMA cycle time (ns)
    uint16_t rec_dma_cycle_time;     // 66: Recommended multiword DMA cycle time
    uint16_t min_pio_cycle_time;     // 67: Min PIO cycle time without flow control
    uint16_t min_pio_iordy_time;     // 68: Min PIO cycle time with IORDY
    uint16_t rsv7[6];                // 69-74: Reserved
    uint16_t queue_depth;            // 75: Queue depth (bits 4:0)
    uint16_t sata_capabilities;      // 76: SATA capabilities
    uint16_t sata_capabilities2;     // 77: SATA additional capabilities
    uint16_t sata_features_supp;     // 78: SATA features supported
    uint16_t sata_features_en;       // 79: SATA features enabled
    uint16_t major_version;          // 80: ATA major version
    uint16_t minor_version;          // 81: ATA minor version
    uint16_t cmd_set0;               // 82: Command set supported 0
    uint16_t cmd_set1;               // 83: Command set supported 1
    uint16_t cmd_set2;               // 84: Command set supported 2
    uint16_t cmd_set_en0;            // 85: Command set enabled 0
    uint16_t cmd_set_en1;            // 86: Command set enabled 1
    uint16_t cmd_set_en2;            // 87: Command set enabled 2
    uint16_t dma_ultra;              // 88: Ultra DMA modes
    uint16_t rsv8[4];                // 89-92: Reserved / erase times
    uint16_t hw_reset_result;        // 93: Hardware reset result
    uint16_t acoustic;               // 94: Acoustic management
    uint16_t rsv9[5];                // 95-99: Reserved
    uint64_t lba48_sectors;          // 100-103: Total LBA48 addressable sectors
    uint16_t rsv10[2];               // 104-105: Reserved
    uint16_t physical_logical;       // 106: Physical/logical sector size
    uint16_t rsv11[9];               // 107-115: Reserved
    uint16_t logical_sector_size[2]; // 116-117: Logical sector size (words)
    uint16_t rsv12[8];               // 118-125: Reserved
    uint16_t last_lun;               // 126: Reserved
    uint16_t rsv13;                  // 127: Reserved
    uint16_t security_status;        // 128: Security status
    uint16_t vendor1[31];            // 129-159: Vendor specific
    uint16_t cfa_power;              // 160: CFA power mode
    uint16_t rsv14[7];               // 161-167: Reserved
    uint16_t form_factor;            // 168: Device form factor
    uint16_t rsv15[3];               // 169-171: Reserved
    char media_serial[60];           // 172-191: Current media serial (ASCII)
    uint16_t sct_cmd_transport;      // 206: SCT command transport
    uint16_t rsv16[5];               // 207-211: Reserved
    uint16_t alignment;              // 209: Alignment of logical in physical
    uint16_t rsv17[6];               // 210-215: Reserved
    uint16_t nv_cache_caps;          // 214: NV cache capabilities
    uint32_t nv_cache_size;          // 215-216: NV cache size (logical blocks)
    uint16_t nv_cache_opts;          // 217: NV cache options
    uint16_t rsv18[3];               // 218-220: Reserved
    uint16_t transport_major;        // 221: Transport major version
    uint16_t transport_minor;        // 222: Transport minor version
    uint16_t rsv19[31];              // 223-253: Reserved
    uint16_t integrity;              // 255: Integrity word (checksum)
} PACKED sata_identify_packet_t;

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
    uint32_t cmdslots = (aport->host_priv->abar->cap & 0x0F00) >> 8;
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
    port->is = 0xFFFFFFFF;
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
        if (port->is & (1 << 30))
        {
            return 3;
        }
    }

    if (port->is & (1 << 30))
    {
        return 3;
    }
    return KERRNO_SUCCESSES;
}

extern void hexdump(void *ptr, size_t len, size_t size);

status_t sata_init_port(pci_device_t *pdev, ahci_host_priv_t *host, ahci_port_priv_t *aport, device_t **out_dev)
{
    sata_port_t *sata_aport = kmalloc(sizeof(sata_port_t));
    memset(sata_aport, 0, sizeof(sata_port_t));
    sata_aport->abar = host->abar;
    sata_aport->ahci_port = aport;
    aport->port_priv = sata_aport;

    sata_identify_packet_t *info_virt = kmalloc(sizeof(sata_identify_packet_t));
    memset(info_virt, 0, sizeof(sata_identify_packet_t));

    ahci_initialize_aport(aport);
    sata_identify_aport(sata_aport, aport, (vaddr_t)info_virt);

    device_t *dev = device_create();
    *out_dev = dev;

    aport->drive = host->ahci_devices_count;

    hexdump(info_virt, sizeof(sata_identify_packet_t), 16);

    trace_debug(MODULE, "getting name");
    char *name = (char *)kmalloc(41);
    memset(name, 0, 41);
    for (int i = 0; i < 40; i += 2)
    {
        if (info_virt->model[i] == ' ' && info_virt->model[i + 1] == ' ')
        {
            break;
        }
        name[i] = info_virt->model[i + 1];
        name[i + 1] = info_virt->model[i];
    }

    dev->drv_data = aport;
    dev->class = DEVICE_BLOCK;
    // dev->read = ahci_read;
    // dev->write = ahci_write;
    dev->init_name = "sata";
    // dev->flags = DEVICE_FLAG_RW | DEVICE_FLAG_BLOCKDEV;
    device_register_child(dev, &pdev->dev);

    // partition_block_device_register(dev);

    uint64_t total_sectors = info_virt->lba28_sectors;
    if (info_virt->cmd_set_en1 & (1 << 10))
    {
        total_sectors = info_virt->lba48_sectors;
    }

    // printf("Detected SATA drive: %s (%u MiB)\n", modelName, info.total_sectors / 2048);
    trace_info(MODULE, "Detected SATA drive: %s (%u MiB)", name, total_sectors / 2048);

    kfree(info_virt);
    return KERRNO_SUCCESSES;
}

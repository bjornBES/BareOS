/*
 * File: ahci.c
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ahci.h"
#include "debug/debug.h"
#include "task/sync/mutex.h"
#include "task/sync/semaphore.h"
#include "partition_manager/partition_manager.h"
#include "device/device.h"
#include "mm/ioremap/ioremap.h"
#include "mm/mmu/mmu.h"
#include "mm/memdefs.h"

#include "stdio.h"
#include "kernel/memory.h"
#include "kernel/ivt.h"
#include "kernel/irq.h"
#include "kernel/cpu.h"

#include "errno/errno.h"

#include <stdbool.h>

#define MODULE                 "AHCI"

#define SATA_SIG_ATA           0x00000101 // SATA drive
#define SATA_SIG_ATAPI         0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB          0xC33C0101 // Enclosure management bridge
#define SATA_SIG_PM            0x96690101 // Port multiplier

#define AHCI_DEV_NULL          0
#define AHCI_DEV_SATA          1
#define AHCI_DEV_SEMB          2
#define AHCI_DEV_PM            3
#define AHCI_DEV_SATAPI        4

#define HBA_PORT_IPM_ACTIVE    1
#define HBA_PORT_DET_PRESENT   3

#define HBA_DET_PRESENT        3
#define HBA_IPM_ACTIVE         1
#define HBA_CMD_CR             (1 << 15)
#define HBA_CMD_FR             (1 << 14)
#define HBA_CMD_FRE            (1 << 4)
#define HBA_CMD_SUD            (1 << 1)
#define HBA_CMD_ST             (1)

#define SATA_READ_DMA_EX       0x25
#define SATA_WRITE_DMA_EX      0x35
#define SATA_IDENTIFY_DEVICE   0xEC
#define SATA_BUSY              0x80
#define SATA_DRQ               0x08

#define HBA_PxCMD_ST           0x0001
#define HBA_PxCMD_FRE          0x0010
#define HBA_PxCMD_FR           0x4000
#define HBA_PxCMD_CR           0x8000

#define MAX_PORTS              4

#define PORT_IRQ_COLD_PRES     BIT(31) /* cold presence detect */
#define PORT_IRQ_TF_ERR        BIT(30) /* task file error */
#define PORT_IRQ_HBUS_ERR      BIT(29) /* host bus fatal error */
#define PORT_IRQ_HBUS_DATA_ERR BIT(28) /* host bus data error */
#define PORT_IRQ_IF_ERR        BIT(27) /* interface fatal error */
#define PORT_IRQ_IF_NONFATAL   BIT(26) /* interface non-fatal error */
#define PORT_IRQ_OVERFLOW      BIT(24) /* xfer exhausted available S/G */
#define PORT_IRQ_BAD_PMP       BIT(23) /* incorrect port multiplier */

#define PORT_IRQ_PHYRDY        BIT(22) /* PhyRdy changed */
#define PORT_IRQ_DMPS          BIT(7)  /* mechanical presence status */
#define PORT_IRQ_CONNECT       BIT(6)  /* port connect change status */
#define PORT_IRQ_SG_DONE       BIT(5)  /* descriptor processed */
#define PORT_IRQ_UNK_FIS       BIT(4)  /* unknown FIS rx'd */
#define PORT_IRQ_SDB_FIS       BIT(3)  /* Set Device Bits FIS rx'd */
#define PORT_IRQ_DMAS_FIS      BIT(2)  /* DMA Setup FIS rx'd */
#define PORT_IRQ_PIOS_FIS      BIT(1)  /* PIO Setup FIS rx'd */
#define PORT_IRQ_D2H_REG_FIS   BIT(0)  /* D2H Register FIS rx'd */

#define PORT_IRQ_FREEZE        (PORT_IRQ_HBUS_ERR | PORT_IRQ_IF_ERR | PORT_IRQ_CONNECT | PORT_IRQ_PHYRDY | PORT_IRQ_UNK_FIS | PORT_IRQ_BAD_PMP)
#define PORT_IRQ_ERROR         (PORT_IRQ_FREEZE | PORT_IRQ_TF_ERR | PORT_IRQ_HBUS_DATA_ERR)
#define DEF_PORT_IRQ           (PORT_IRQ_ERROR | PORT_IRQ_SG_DONE | PORT_IRQ_SDB_FIS | PORT_IRQ_DMAS_FIS | PORT_IRQ_PIOS_FIS | PORT_IRQ_D2H_REG_FIS)

// 32 slots × 256 bytes each = 8KB total (2 pages)
// 256 gives room for CFIS(128) + 8 PRDTs(128) with alignment headroom
#define CTBA_SLOT_SIZE 256

typedef struct
{
    struct
    {
        pci_caps_entry_header pm_id;
        uint16_t pm_caps;
        uint16_t pm_cs;
    } pm;

    struct
    {
        pci_caps_entry_header msi_id;
        uint16_t msi_caps;
        uint8_t msi_addr_off;
        uint8_t msi_data_off;
        uint8_t msi_addru_off;
    } msi;
} ahci_caps;

typedef struct
{
    semaphore_t done; // starts at 0 — sem_wait blocks until ISR posts it
    int status;       // error code / result, set by ISR before posting
} ahci_cmd_ctx_t;

typedef struct ahci_port
{
    HBA_MEM *abar;
    HBA_PORT *port;
    void *clb;
    void *fb;
    mutex_t slot_lock;      // protects slot_bitmap
    semaphore_t slots_free; // count of available slots, init = num_slots
    uint32_t slot_bitmap;   // bit set = slot in use
    uint32_t outstanding_mask;
    uint32_t command_issue;
    ahci_cmd_ctx_t cmds[32];
    void *ctba[32];
    void *unused[28]; // Even out the data size to 256 bytes
} ahci_port_t;

typedef struct
{
    uint8_t command; // 0xEC for IDENTIFY DEVICE, 0x25 for READ DMA EXT, etc.
    uint64_t lba;
    uint16_t sector_count;
    bool is_write;
    void *buffer;       // where the X-bytes result lands
    size_t buffer_size; // the X-bytes
} ahci_request_t;

int ahci_indexes[32];
HBA_MEM *ahci_abar;
ahci_port_t *ports;
int ahci_devices_count;

uint32_t ahci_find_cmdslot(ahci_port_t *aport)
{
    HBA_PORT *port = aport->port;
    uint32_t slots = (port->sact | port->ci);
    uint32_t cmdslots = (aport->abar->cap & 0x0F00) >> 8;
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

void ahci_start_cmd(HBA_PORT *port)
{
    while (port->cmd & HBA_PxCMD_CR);

    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

void ahci_stop_cmd(HBA_PORT *port)
{
    port->cmd &= ~HBA_PxCMD_ST;

    port->cmd &= ~HBA_PxCMD_FRE;

    while ((port->cmd & HBA_CMD_FR) || (port->cmd & HBA_CMD_CR));
}

void ahci_program_command_slot(ahci_port_t *aport, uint32_t slot, ahci_request_t *req)
{
    HBA_PORT *port = aport->port;
    port->is = (uint32_t)-1; // Clear pending interrupt bits

    HBA_CMD_HEADER *cmd_header = (HBA_CMD_HEADER *)aport->clb;
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmd_header->w = req->is_write;

    HBA_CMD_TBL *cmd_table = (HBA_CMD_TBL *)aport->ctba[slot];

    uint32_64 dba_phys = (uint32_64)req->buffer;
    cmd_table->prdt_entry[0].dba = dba_phys & 0xFFFFFFFF;
#ifdef __x86_64__
    cmd_table->prdt_entry[0].dbau = (dba_phys >> 32);
#else
    cmd_table->prdt_entry[0].dbau = 0;
#endif
    cmd_table->prdt_entry[0].dbc = req->buffer_size - 1;
    cmd_table->prdt_entry[0].i = 1;

    FIS_REG_H2D *cmd_fis = (FIS_REG_H2D *)(&cmd_table->cfis);
    cmd_fis->fis_type = FIS_TYPE_REG_H2D; // Host to device
    cmd_fis->c = 1;
    cmd_fis->command = req->command;

    cmd_fis->device = 1 << 6;
    cmd_fis->lba0 = (uint8_t)(req->lba & 0xFF);
    cmd_fis->lba1 = (uint8_t)((req->lba >> 8) & 0xFF);
    cmd_fis->lba2 = (uint8_t)((req->lba >> 16) & 0xFF);

    cmd_fis->lba3 = (uint8_t)((req->lba >> 24) & 0xFF);
    cmd_fis->lba4 = (uint8_t)((req->lba >> 32) & 0xFF);
    cmd_fis->lba5 = (uint8_t)((req->lba >> 40) & 0xFF);

    cmd_fis->countl = (req->sector_count & 0xFF);
    cmd_fis->counth = (req->sector_count >> 8);
}

int ahci_check_type(HBA_PORT *port)
{
    uint32_t ssts = port->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) // Check drive status
    {
        return AHCI_DEV_NULL;
    }
    if (ipm != HBA_PORT_IPM_ACTIVE)
    {
        return AHCI_DEV_NULL;
    }

    switch (port->sig)
    {
        case SATA_SIG_ATAPI :
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB :
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM :
            return AHCI_DEV_PM;
        default :
            return AHCI_DEV_SATA;
    }
}

void ahci_initialize_port(ahci_port_t *aport)
{
    HBA_PORT *port = aport->port;
    mutex_init(&aport->slot_lock);
    sem_init(&aport->slots_free, 32);
    aport->slot_bitmap = 0;
    for (int i = 0; i < 32; i++)
    {
        sem_init(&aport->cmds[i].done, 0); // 0 = nothing completed yet
    }

    port->ie = BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(22) | BIT(23) | BIT(24) | BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30);

    ahci_stop_cmd(port);

    void *mapped_clb_virt;
    void *mapped_clb = kmalloc_phys(4096, &mapped_clb_virt);
    port->clb = (uint32_t)((uint32_64)mapped_clb & 0xFFFFFFFF);
#ifdef __x86_64__
    port->clbu = (uint32_t)((uint32_64)mapped_clb >> 32);
#else
    port->clbu = 0;
#endif
    aport->clb = mapped_clb_virt;

    void *mapped_fb_virt;
    void *mapped_fb = kmalloc_phys(4096, &mapped_fb_virt);
    port->fb = (uint32_t)((uint32_64)mapped_fb & 0xFFFFFFFF);
#ifdef __x86_64__
    port->clbu = (uint32_t)((uint32_64)mapped_fb >> 32);
#else
    port->clbu = 0;
#endif
    aport->fb = mapped_fb_virt;

    HBA_CMD_HEADER *cmd_header = (HBA_CMD_HEADER *)mapped_clb_virt;

    void *ctba_pool_virt;
    void *ctba_pool = kcalloc_phys(1, 32 * CTBA_SLOT_SIZE, &ctba_pool_virt);

    for (uint8_t i = 0; i < 32; i++)
    {
        void *ctba_buf = (void *)((uintptr_t)ctba_pool + i * CTBA_SLOT_SIZE);
        void *ctba_buf_virt = (void *)((uintptr_t)ctba_pool_virt + i * CTBA_SLOT_SIZE);
        cmd_header[i].prdtl = 1;
        aport->ctba[i] = ctba_buf_virt;
        cmd_header[i].ctba = (uint32_64)ctba_buf & 0xFFFFFFFF;
#ifdef __x86_64__
        cmd_header[i].ctbau = ((uint32_64)ctba_buf >> 32);
#else
        cmd_header[i].ctbau = 0;
#endif
    }

    ahci_start_cmd(port);
}

void ahci_ring_doorbell(ahci_port_t *aport, int slot)
{
    ENTER_FUNC(MODULE, "%p, %u", aport, slot);
    // PxCI (Command Issue) register — set the bit for this slot
    log_debug(MODULE, "now in port->ci = 0x%x", aport->port->ci);
    aport->command_issue |= BIT(slot);
    aport->outstanding_mask |= BIT(slot);
    aport->port->ci |= BIT(slot);
    log_debug(MODULE, "now in port->ci = 0x%x", aport->port->ci);
}

uint32_t ahci_read_completed_mask(ahci_port_t *aport)
{
    uint32_t ci_now = aport->port->ci;
    uint32_t completed = aport->outstanding_mask & ~ci_now; // was outstanding, no longer in PxCI
    aport->outstanding_mask &= ~completed;                  // remove them from tracking
    return completed;
}

int ahci_read_slot_status(ahci_port_t *aport, int slot)
{
    uint32_t is = aport->port->is;

    if (is & PORT_IRQ_TF_ERR)
    {
        uint8_t err = (aport->port->tfd >> 8) & 0xFF;
        return err;
    }

    return 0; // success
}

int ahci_submit(ahci_port_t *aport, ahci_request_t *req)
{
    sem_wait(&aport->slots_free); // blocks if all slots are busy — this is the backpressure

    mutex_lock(&aport->slot_lock);
    int slot = 0xFFFFFFFF;
    {
        for (uint32_t i = 0; i < sizeof(uint32_t) * 8; i++)
        {
            if ((((aport->slot_bitmap) >> (i)) & 1) == 0)
            {
                slot = i;
                break;
            }
        }
    }
    if (slot == 0xFFFFFFFF)
    {
        slot = ahci_find_cmdslot(aport);
    }
    aport->slot_bitmap |= BIT(slot);
    mutex_unlock(&aport->slot_lock);

    ahci_program_command_slot(aport, slot, req); // your existing register-poking code
    ahci_ring_doorbell(aport, slot);

    sem_wait(&aport->cmds[slot].done); // blocks here until the ISR posts it

    int status = aport->cmds[slot].status;

    mutex_lock(&aport->slot_lock);
    aport->slot_bitmap &= ~(1 << slot);
    mutex_unlock(&aport->slot_lock);

    sem_post(&aport->slots_free); // free the slot back to the pool

    return status;
}

int ahci_identify_device(ahci_port_t *aport, void *buf, void *buf_virt)
{
    log_debug(MODULE, "ahci_identify_device(%p, %p)", aport, buf);
    log_debug(MODULE, "buf = p%p / v%p", buf, buf_virt);

    ahci_request_t req = {
        .command = SATA_IDENTIFY_DEVICE, // IDENTIFY DEVICE
        .lba = 0,                        // not used by this command
        .sector_count = 0,               // not used by this command
        .is_write = false,               // it's a read device sends data to us
        .buffer = buf,
        .buffer_size = 512,              // IDENTIFY always returns exactly one 512-byte sector
    };
    return ahci_submit(aport, &req);
}

int ahci_read_sectors_command(ahci_port_t *aport, uint64_t sector, size_t count, uint16_t *buf)
{
    ahci_request_t req = {
        .command = SATA_READ_DMA_EX,
        .lba = sector,
        .is_write = false,
        .buffer = buf,
        .sector_count = count,
        .buffer_size = (count * 512), // 8K bytes
    };
    return ahci_submit(aport, &req);
}

int ahci_write_sectors_command(ahci_port_t *aport, uint64_t sector, size_t count, uint16_t *buf)
{
    ahci_request_t req = {
        .command = SATA_WRITE_DMA_EX,
        .lba = sector,
        .is_write = true,
        .buffer = buf,
        .sector_count = count,
        .buffer_size = (count * 512), // 8K bytes
    };
    return ahci_submit(aport, &req);
}

ssize_t ahci_read(void *buffer, off_t offset, size_t count, device_t *device)
{
    // log_debug(MODULE, "ahci_read(%p, %u, %u, %p)", buffer, offset, count, device);
    uint64_t sector = (uint64_t)offset;
    sata_private_data *priv = (sata_private_data *)device->priv;
    uint16_t drive = priv->drive;
    ahci_port_t *aport = &ports[drive];
    if (aport->abar != 0)
    {
        uint16_t *u16Buffer = (uint16_t *)mmu_arch_virt_to_phys(&kernel_page, (vaddr_t)buffer);
        int state = ahci_read_sectors_command(aport, sector, count, u16Buffer);
        if (state == 0)
        {
            return count;
        }
        return state;
    }
    else
    {
        return -1;
    }
    return -1;
}

ssize_t ahci_write(void *buffer, off_t offset, size_t count, device_t *device)
{
    uint64_t sector = (uint64_t)offset;
    sata_private_data *priv = (sata_private_data *)device->priv;
    uint16_t drive = priv->drive;
    ahci_port_t *aport = &ports[drive];
    if (aport->abar != 0)
    {
        uint16_t *u16Buffer = (uint16_t *)mmu_arch_virt_to_phys(&kernel_page, (vaddr_t)buffer);
        int state = ahci_write_sectors_command(aport, sector, count, u16Buffer);
        if (state == 0)
        {
            return count;
        }
        return state;
    }
    else
    {
        return -1;
    }
    return -1;
}

void ahci_handle_port_error(ahci_port_t *port)
{
    // todo handle_port_error
    FUNC_NOT_IMPLEMENTED(MODULE);
}

void ahci_isr_port_handler(ahci_port_t *aport)
{
    uint32_t completed = ahci_read_completed_mask(aport); // which slots finished, from port regs
    log_debug(MODULE, "completed = %u", completed);
    for (int slot = 0; slot < 32; slot++)
    {
        if ((completed & (BIT(slot))) == 0)
        {
            continue;
        }

        log_debug(MODULE, "found slot %u", slot);
        aport->cmds[slot].status = ahci_read_slot_status(aport, slot);
        sem_post(&aport->cmds[slot].done); // wakes whichever thread called ahci_submit for this slot
    }
}

int ahci_isr_handler(intr_frame_t *frame)
{
    ENTER_FUNC(MODULE, "%p", frame);
    irq_arch_disable();
    uint32_t is = ahci_abar->is;
    log_debug(MODULE, "is = 0b%b", is);
    uint8_t num_ports = (ahci_abar->cap & 0x1F) + 1;
    for (size_t i = 0; i < num_ports; i++)
    {
        if (BIT_GET(is, i) == 0)
        {
            continue;
        }
        int aport_index = ahci_indexes[i];

        ahci_port_t *port = &ports[aport_index];
        // todo
        if (port->port == NULL)
        {
            log_err(MODULE, "port %u isn't ready", i);
            continue;
        }
        uint32_t port_is = port->port->is;
        if ((port_is & PORT_IRQ_ERROR) != 0)
        {
            log_err(MODULE, "port %u @ %p has errors", i, port);
            continue;
            // ahci_handle_port_error(port);
        }
        ahci_isr_port_handler(port);
    }
    lapic_eoi();
    irq_arch_enable();
    return RETURN_GOOD;
}

void ahci_initialize_abar(HBA_MEM *abar, pci_device_id *pdev, ahci_caps caps)
{
    ahci_abar = abar;
    if (caps.msi.msi_id.id == 0x5)
    {
        pci_caps_write_dword(pdev, caps.msi.msi_id, caps.msi.msi_addr_off, (0xFEE << 20) | (cpu_arch_get_current()->apic_id << 12));
        pci_caps_write_dword(pdev, caps.msi.msi_id, caps.msi.msi_data_off, PIC_MSI_VEC1);
        uint32_t con = pci_caps_read_dword(pdev, caps.msi.msi_id, 0);
        pci_caps_write_dword(pdev, caps.msi.msi_id, 0, con | 0x00010000);
    }
    uint32_t pi = abar->pi;
    log_debug(MODULE, "pi = 0b%b", pi);
    uint8_t num_ports = (abar->cap & 0x1F) + 1;
    log_debug(MODULE, "np = %d", num_ports);
    abar->ghc |= BIT(1);

    for (size_t i = 0; i < num_ports; i++)
    {
        if (pi & 1)
        {
            int dt = ahci_check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                log_debug(MODULE, "after");
                sata_identify_packet *info_virt = NULL;
                log_debug(MODULE, "here");
                sata_identify_packet *info = kmalloc_phys(sizeof(sata_identify_packet), (void **)&info_virt);
                log_debug(MODULE, "here1");
                log_debug(MODULE, "info_virt = %p, info = %p", info_virt, info);

                allocator_print_status();
                allocator_print_blocks();

                HBA_PORT *port = &abar->ports[i];
                log_debug(MODULE, "found sata");
                ports[ahci_devices_count].abar = abar;
                ports[ahci_devices_count].port = port;
                ahci_indexes[i] = ahci_devices_count;
                ahci_port_t *aport = &ports[ahci_devices_count];
                ahci_initialize_port(aport);
                log_debug(MODULE, "Port %u initialized", ahci_devices_count);
                ahci_identify_device(aport, info, info_virt);
                log_debug(MODULE, "Port %u identified", ahci_devices_count);

                device_t *dev = (device_t *)malloc(sizeof(device_t));
                sata_private_data *priv = (sata_private_data *)malloc(sizeof(sata_private_data));
                priv->drive = ahci_devices_count;

                dev->priv = priv;
                dev->type = DEVICE_BLOCK;
                dev->read = ahci_read;
                dev->write = ahci_write;
                dev->class_name = "sata";
                dev->flags = DEVICE_FLAG_RW | DEVICE_FLAG_BLOCKDEV;
                device_register(dev);

                partition_block_device_register(dev);

                uint64_t total_sectors = info_virt->lba28_sectors;
                if (info_virt->cmd_set_en1 & (1 << 10))
                {
                    total_sectors = info_virt->lba48_sectors;
                }

                // printf("Detected SATA drive: %s (%u MiB)\n", modelName, info.total_sectors / 2048);
                log_info(MODULE, "Detected SATA drive: %s (%u MiB)", info_virt->model, total_sectors / 2048);
                ahci_devices_count++;

                free(info_virt);
            }
        }
    }
}

void ahci_initialize(pci_device_id *pdev)
{
    ENTER_FUNC(MODULE, "%p", pdev);
    log_debug(MODULE, "bus: 0x%x, slot: 0x%x, function: 0x%x", pdev->bus, pdev->slot, pdev->function);
    log_debug(MODULE, "device_id: 0x%x, vendor: 0x%x", pdev->device_id, pdev->vendor_id);
    log_debug(MODULE, "status: 0x%x, command: 0x%x", pdev->status, pdev->command);
    log_debug(MODULE, "class_code: 0x%x, sub_class: 0x%x, prog_if: 0x%x, revision_id: 0x%x", pdev->class_code, pdev->sub_class, pdev->prog_if, pdev->revision_id);
    log_debug(MODULE, "bist: 0x%x, header_type: 0x%x, latency_timer: 0x%x, cache_line_size: 0x%x", pdev->bist, pdev->header_type, pdev->latency_timer, pdev->cache_line_size);
    pci_header0 *header = &pdev->header.header0;
    log_debug(MODULE, "bar0: 0x%x, bar1: 0x%x, bar2: 0x%x, bar3: 0x%x, bar4: 0x%x, bar5: 0x%x", header->bar0, header->bar1, header->bar2, header->bar3, header->bar4, header->bar5);
    log_debug(MODULE, "card_bus_cis: 0x%x, subsystem ident: 0x%x_%x, rom: 0x%x", header->card_bus_cis, header->subsystem_id, header->subsystem_vendor_id, header->rom_base_address);
    log_debug(MODULE, "capabilities ptr: 0x%x, interrupt information: 0x%x_%x", header->capabilities_ptr, header->interrupt_pin, header->interrupt_line);
    log_debug(MODULE, "max_latency: 0x%x, min_grant: 0x%x", header->max_latency, header->min_grant);
    ports = (ahci_port_t *)malloc(sizeof(ahci_port_t) * MAX_PORTS);
    ahci_caps caps;
    pci_caps_entry_header prev;
    prev.next = 0;
    uint8_t new_offset = header->capabilities_ptr;
    ivt_arch_set_handler(PIC_MSI_VEC1, ahci_isr_handler);
    while (true)
    {
        log_debug(MODULE, "next offset at 0x%x", new_offset);
        pci_caps_entry_header entry = pci_read_caps_entry_header(pdev, new_offset);
        if (entry.id == 0)
        {
            break;
        }
        uint8_t old_offset = prev.next;
        log_debug(MODULE, "got caps id 0x%x, next 0x%x", entry.id, entry.next);
        switch (entry.id)
        {
            case 0x1 :
                {
                    caps.pm.pm_id = entry;
                    caps.pm.pm_caps = pci_config_read_word(pdev->bus, pdev->slot, pdev->function, old_offset + 2);
                    caps.pm.pm_cs = pci_config_read_word(pdev->bus, pdev->slot, pdev->function, old_offset + 4);
                }
                break;
            case 0x5 :
                {
                    caps.msi.msi_id = entry;
                    caps.msi.msi_caps = pci_config_read_dword(pdev->bus, pdev->slot, pdev->function, caps.msi.msi_id.this + 2) >> 16;
                    caps.msi.msi_addr_off = 0x4;
                    caps.msi.msi_data_off = 0xC;
                    if (BIT_GET(caps.msi.msi_caps, 7) == 1)
                    {
                        caps.msi.msi_addru_off = 0x8;
                    }
                }
                break;

            default :
                log_err(MODULE, "missed 0x%x", entry.id);
                break;
        }
        new_offset = entry.next;
        if (new_offset == 0)
        {
            break;
        }
    }

    paddr_t bar5 = (paddr_t)pdev->header.header0.bar5;
    vaddr_t vbar5 = ioremap(bar5, sizeof(HBA_MEM));
    // paging_alloc_frame_region(bar5, (size_t)bar5 + sizeof(HBA_MEM));

    mmu_map_region(&kernel_page, vbar5, bar5, sizeof(HBA_MEM), mmio_flags);

    ahci_initialize_abar((HBA_MEM *)vbar5, pdev, caps);
    log_info(MODULE, "exit out");
}

/*
 * File: ahci.c
 * File Created: 26 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/ahci/ahci.h"

#include "asm/mmu_arch.h"

#include "drivers/driver.h"

#include "sync/mutex.h"
#include "sync/semaphore.h"

#include "pci/pci.h"

#include "irq/irq.h"
#include "cpu/cpu.h"

#include "kerrno.h"

#include "memory.h"
#include "string.h"

#include "sata.h"

#define MODULE                 "ahci-driver"

#define DRV_NAME               "ahci"

#define DEFAULT_BAR_INDEX      5

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

#define HBA_PxCMD_ST           0x0001
#define HBA_PxCMD_FRE          0x0010
#define HBA_PxCMD_FR           0x4000
#define HBA_PxCMD_CR           0x8000

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

void ahci_start_cmd(hba_port_t *port)
{
    while (port->cmd & HBA_PxCMD_CR);

    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

void ahci_stop_cmd(hba_port_t *port)
{
    port->cmd &= ~HBA_PxCMD_ST;

    port->cmd &= ~HBA_PxCMD_FRE;

    while ((port->cmd & HBA_CMD_FR) || (port->cmd & HBA_CMD_CR));
}

void ahci_initialize_aport(ahci_port_priv_t *aport)
{
    hba_port_t *port = aport->port;

#if CONFIG_MUTEX == 1
    mutex_init(&aport->slot_lock);
#endif
#if CONFIG_SEMAPHORE == 1
    sem_init(&aport->slots_free, 32);
#endif

    aport->slot_bitmap = 0;
#if CONFIG_DMA == 1
    for (int i = 0; i < 32; i++)
    {
        #if CONFIG_SEMAPHORE == 1
        sem_init(&aport->cmds[i].done, 0); // 0 = nothing completed yet
        #endif
        aport->cmds[i].dma = dma_allocate(MAX_TRANSFER_SIZE);
        if (aport->cmds[i].dma == NULL)
        {
            KERNEL_PANIC(MODULE, "OMM");
        }
    }
#endif

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

    ahci_cmd_header_t *cmd_header = (ahci_cmd_header_t *)mapped_clb_virt;

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

status_t ahci_program_command_slot(ahci_port_priv_t *aport, uint32_t slot, ahci_cmd_request_t *req)
{
    ENTER_FUNC("%p, %u, %p", aport, slot, req);
    hba_port_t *port = aport->port;
    port->is = (uint32_t)-1; // Clear pending interrupt bits

#if CONFIG_DMA == 1
    if (req->is_write == true)
    {
        dma_copy_from(aport->cmds[slot].dma, (void *)req->buffer, req->buffer_size);
    }
#endif

    ahci_cmd_header_t *cmd_header = (ahci_cmd_header_t *)aport->clb;
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmd_header->w = req->is_write;
    cmd_header->prdtl = (uint16_t)((req->sector_count - 1) >> 4) + 1;

    HBA_CMD_TBL *cmd_table = (HBA_CMD_TBL *)aport->ctba[slot];
    memset(cmd_table, 0, sizeof(HBA_CMD_TBL) + (cmd_header->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

#if CONFIG_DMA == 1
    if (req->buffer_size >= aport->cmds[slot].dma->size)
    {
        log_crit(MODULE, "can't read more then the allocated size");
        KERNEL_PANIC(MODULE, "TODO"); // TODO
    }

    paddr_t dba_phys = aport->cmds[slot].dma->phys;
#else
    paddr_t dba_phys = mmu_arch_virt_to_phys(&kernel_page, req->buffer);
    trace_debug(MODULE, "dba_phys @ %p", dba_phys);
    // paging_print_tree(&kernel_page);
#endif
    size_t count = req->sector_count;
    int i;
    for (i = 0; i < cmd_header->prdtl - 1; i++)
    {
        cmd_table->prdt_entry[i].dba = dba_phys & 0xFFFFFFFF;
#ifdef __x86_64__
        cmd_table->prdt_entry[i].dbau = (dba_phys >> 32);
#else
        cmd_table->prdt_entry[i].dbau = 0;
#endif
        cmd_table->prdt_entry[i].dbc = 8 * 1024 - 1;
        cmd_table->prdt_entry[i].i = 1;
        dba_phys += 8 * 1024;
        count -= 16;
    }

    cmd_table->prdt_entry[i].dba = dba_phys & 0xFFFFFFFF;
#ifdef __x86_64__
    cmd_table->prdt_entry[i].dbau = (dba_phys >> 32);
#else
    cmd_table->prdt_entry[i].dbau = 0;
#endif
    cmd_table->prdt_entry[i].dbc = (count << 9) - 1;
    cmd_table->prdt_entry[i].i = 1;

    FIS_REG_H2D *cmd_fis = (FIS_REG_H2D *)(&cmd_table->cfis);
    cmd_fis->fis_type = FIS_TYPE_REG_H2D; // Host to device
    cmd_fis->c = 1;
    cmd_fis->command = req->command;

    cmd_fis->lba0 = (uint8_t)(req->lba & 0xFF);
    cmd_fis->lba1 = (uint8_t)((req->lba >> 8) & 0xFF);
    cmd_fis->lba2 = (uint8_t)((req->lba >> 16) & 0xFF);
    cmd_fis->device = 1 << 6;

    cmd_fis->lba3 = (uint8_t)((req->lba >> 24) & 0xFF);
    cmd_fis->lba4 = (uint8_t)((req->lba >> 32) & 0xFF);
    cmd_fis->lba5 = (uint8_t)((req->lba >> 40) & 0xFF);

    cmd_fis->countl = (req->sector_count & 0xFF);
    cmd_fis->counth = (req->sector_count >> 8);

    // ENTER_FUNC(MODULE, "%p, %u", aport, slot);
    // PxCI (Command Issue) register — set the bit for this slot

    aport->outstanding_mask |= BIT(slot);
    // trace_debug(MODULE, "slot = %u", slot);
    // trace_debug(MODULE, "read back high: %u low: %u", cmd_fis->counth, cmd_fis->countl);
    aport->port->ci |= BIT(slot);

#if CONFIG_SEMAPHORE == 1
    sem_wait(&aport->cmds[slot].done); // blocks here until the ISR posts it
#endif

#if CONFIG_DMA == 1
    if (req->is_write == false)
    {
        dma_copy_to(aport->cmds[slot].dma, (void *)req->buffer, req->buffer_size);
    }
#endif
    return KERRNO_SUCCESSES;
}

status_t ahci_submit(ahci_port_priv_t *aport, ahci_cmd_request_t *req)
{
    ENTER_FUNC("%p, %p", aport, req);
#if CONFIG_SEMAPHORE == 1
    sem_wait(&aport->slots_free); // blocks if all slots are busy — this is the backpressure
#endif

#if CONFIG_MUTEX == 1
    mutex_lock(&aport->slot_lock);
#endif

    uint32_t slot = (~0);
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
    if (slot == -1)
    {
        if (aport->port_type == AHCI_DEV_SATA)
        {
            slot = sata_find_cmdslot(aport);
        }
    }
    aport->slot_bitmap |= BIT(slot);

#if CONFIG_MUTEX == 1
    mutex_unlock(&aport->slot_lock);
#endif

    ahci_program_command_slot(aport, slot, req); // your existing register-poking code
    status_t status = aport->cmds[slot].status;

#if CONFIG_MUTEX == 1
    mutex_lock(&aport->slot_lock);
#endif
    aport->slot_bitmap &= ~(1 << slot);
#if CONFIG_MUTEX == 1
    mutex_unlock(&aport->slot_lock);
#endif

#if CONFIG_SEMAPHORE == 1
    sem_post(&aport->slots_free); // free the slot back to the pool
#endif

    return status;
}

int ahci_check_type(hba_port_t *port)
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

status_t ahci_init_abar(pci_device_t *dev, ahci_host_priv_t *host)
{
    ENTER_FUNC("%p, %p", dev, host);

    if (dev->msi_cap != 0)
    {
    }

    uint32_t pi = host->abar->pi;
    host->abar->ghc |= HBA_GHC_INTERRUPT_ENABLE;

    for (size_t i = 0; i < host->nports; i++)
    {
        if (pi & 1)
        {
            int dt = ahci_check_type(&host->abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                host->aports[host->ahci_devices_count].port_type = dt;
                host->aports[host->ahci_devices_count].host_priv = host;
                host->aports[host->ahci_devices_count].port = &host->abar->ports[i];
                host->aports[host->ahci_devices_count].ahci_index = i;
                device_t *port_dev;
                host->ahci_devices_count++;
                sata_init_port(dev, host, &host->aports[host->ahci_devices_count - 1], &port_dev);
            }
        }
    }
    return KERRNO_SUCCESSES;
}

status_t ahci_init_irq(pci_device_t *dev, ahci_host_priv_t *host_priv)
{
    size_t nvec = 0;
    status_t ret = 0;
    if (dev->no_msi == 1)
    {
        trace_crit(MODULE, "intx path");
        FUNC_NOT_IMPLEMENTED();
        // INTX path
    }

    ret = pci_init_irq(dev, host_priv->nports, SIZE_MAX, PCI_IRQ_MSI);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    return KERRNO_SUCCESSES;
}

uint32_t ahci_read_completed_mask(ahci_port_priv_t *aport)
{
    uint32_t ci_now = aport->port->ci;
    uint32_t completed = aport->outstanding_mask & ~ci_now; // was outstanding, no longer in PxCI
    aport->outstanding_mask &= ~completed;                  // remove them from tracking
    return completed;
}

status_t ahci_read_slot_status(ahci_port_priv_t *aport, int slot)
{
    uint32_t is = aport->port->is;

    if (is & PORT_IRQ_TF_ERR)
    {
        uint8_t err = (aport->port->tfd >> 8) & 0xFF;
        return err;
    }

    return KERRNO_SUCCESSES; // success
}

void ahci_isr_port_handler(ahci_port_priv_t *aport)
{
    uint32_t completed = ahci_read_completed_mask(aport); // which slots finished, from port regs
    trace_debug(MODULE, "completed = 0x%x", completed);
    for (int slot = 0; slot < 32; slot++)
    {
        if ((completed & (BIT(slot))) == 0)
        {
            continue;
        }

        trace_debug(MODULE, "slot = %u", slot);

        aport->cmds[slot].status = ahci_read_slot_status(aport, slot);
#if CONFIG_SEMAPHORE == 1
        sem_post(&aport->cmds[slot].done);
#endif
    }
}

status_t ahci_handler(pci_device_t *dev)
{
    ENTER_FUNC("%p", dev);
    ahci_host_priv_t *host_priv = dev->priv;

    uint32_t is = host_priv->abar->is;
    // log_debug(MODULE, "is = 0b%b", is);
    uint8_t num_ports = (host_priv->abar->cap & 0x1F) + 1;
    for (size_t i = 0; i < host_priv->ahci_devices_count; i++)
    {
        ahci_port_priv_t *aport = &host_priv->aports[i];
        trace_debug(MODULE, "aport @ %p", aport);
        if (BIT_GET(is, aport->ahci_index) == 0)
        {
            continue;
        }
        if (aport->port == NULL)
        {
            trace_err(MODULE, "port %u isn't ready", aport->ahci_index);
            continue;
        }
        uint32_t port_is = aport->port->is;
        if ((port_is & PORT_IRQ_ERROR) != 0)
        {
            trace_err(MODULE, "port %u @ %p has errors", i, aport);
            continue;
            // ahci_handle_port_error(port);
        }
        ahci_isr_port_handler(aport);
    }

    return KERRNO_SUCCESSES;
}

status_t ahci_init_one(pci_device_t *dev, pci_device_id_t *id)
{
    ENTER_FUNC("%p, %p", dev, id);

    ahci_host_priv_t *host_priv = kmalloc(sizeof(ahci_host_priv_t));
    dev->priv = host_priv;

    status_t ret = 0;

    ret = pci_set_bus_master(dev, 1);
    ret |= pci_set_command_flags(dev, PCI_COMMAND_MEMORY);
    ret |= pci_clear_command_flags(dev, PCI_COMMAND_INTX_DISABLE);

    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }

    uint8_t bar_index = DEFAULT_BAR_INDEX;

    // TODO check vendor/device fields some change the bar index

    pci_bar_t ahci_bar;
    ret = pci_get_bar(dev, bar_index, &ahci_bar);

    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }

    if (!ahci_bar.is_mmio)
    {
        trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", ahci_bar.base, ahci_bar.is_64bit BOOL_TO_STRING, ahci_bar.is_mmio BOOL_TO_STRING, ahci_bar.prefetchable BOOL_TO_STRING, ahci_bar.size);
        KERRNO_RETURN(KERRNO_DONT_DO_THAT, "bar%u is I/O", bar_index);
    }

    ret = pci_map_bar(dev, bar_index, (void *)&host_priv->abar);
    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }
    trace_debug(MODULE, "%p, is 64: %s, is mmio: %s, pre: %s, size: %p", host_priv->abar, ahci_bar.is_64bit BOOL_TO_STRING, ahci_bar.is_mmio BOOL_TO_STRING, ahci_bar.prefetchable BOOL_TO_STRING, ahci_bar.size);

    uint32_t pi = host_priv->abar->pi;
    host_priv->nports = (host_priv->abar->cap & 0x1F) + 1;

    dev->dev.class = DEVICE_BLOCK;
    dev->dev.init_name = "AHCI";

    ahci_init_irq(dev, host_priv);

    interrupt_vector_t vector;
    kernel_irq_t irq;
    ret = pci_get_irq(dev, &vector, &irq);
    dev->handler = ahci_handler;

    return ahci_init_abar(dev, host_priv);
}

void ahci_remove_one(pci_device_t *dev)
{
    return;
}

static pci_device_id_t ahci_pci_tbl[] = {
    /* Intel */
    {
        /* Comet Lake PCH-H RAID */
        PCI_VDEVICE(INTEL, 0x06D6),
    },
    {
        /* ICH6 */
        PCI_VDEVICE(INTEL, 0x2652),
    },
    {
        /* ICH6M */
        PCI_VDEVICE(INTEL, 0x2653),
    },
    {
        /* ICH7 */
        PCI_VDEVICE(INTEL, 0x27C1),
    },
    {
        /* ICH7M */
        PCI_VDEVICE(INTEL, 0x27C5),
    },
    {
        /* ICH7R */
        PCI_VDEVICE(INTEL, 0x27C3),
    },
    {
        /* ULi M5288 */
        PCI_VDEVICE(AL, 0x5288),
    },
    {
        /* ESB2 */
        PCI_VDEVICE(INTEL, 0x2681),
    },
    {
        /* ESB2 */
        PCI_VDEVICE(INTEL, 0x2682),
    },
    {
        /* ESB2 */
        PCI_VDEVICE(INTEL, 0x2683),
    },
    {
        /* ICH7-M DH */
        PCI_VDEVICE(INTEL, 0x27C6),
    },
    {
        /* ICH8 */
        PCI_VDEVICE(INTEL, 0x2821),
    },
    {
        /* ICH8/Lewisburg RAID*/
        PCI_VDEVICE(INTEL, 0x2822),
    },
    {
        /* ICH8 */
        PCI_VDEVICE(INTEL, 0x2824),
    },
    {
        /* ICH8M */
        PCI_VDEVICE(INTEL, 0x2829),
    },
    {
        /* ICH8M */
        PCI_VDEVICE(INTEL, 0x282A),
    },
    {
        /* ICH9 */
        PCI_VDEVICE(INTEL, 0x2922),
    },
    {
        /* ICH9 */
        PCI_VDEVICE(INTEL, 0x2923),
    },
    {
        /* ICH9 */
        PCI_VDEVICE(INTEL, 0x2924),
    },
    {
        /* ICH9 */
        PCI_VDEVICE(INTEL, 0x2925),
    },
    {
        /* ICH9 */
        PCI_VDEVICE(INTEL, 0x2927),
    },
    {
        /* ICH9M */
        PCI_VDEVICE(INTEL, 0x2929),
    },
    {
        /* ICH9M */
        PCI_VDEVICE(INTEL, 0x292A),
    },
    {
        /* ICH9M */
        PCI_VDEVICE(INTEL, 0x292B),
    },
    {
        /* ICH9M */
        PCI_VDEVICE(INTEL, 0x292C),
    },
    {
        /* ICH9M */
        PCI_VDEVICE(INTEL, 0x292F),
    },
    {
        /* ICH9 */
        PCI_VDEVICE(INTEL, 0x294D),
    },
    {
        /* ICH9M */
        PCI_VDEVICE(INTEL, 0x294E),
    },
    {
        /* Tolapai */
        PCI_VDEVICE(INTEL, 0x502A),
    },
    {
        /* Tolapai */
        PCI_VDEVICE(INTEL, 0x502B),
    },
    {
        /* ICH10 */
        PCI_VDEVICE(INTEL, 0x3A05),
    },
    {
        /* ICH10 */
        PCI_VDEVICE(INTEL, 0x3A22),
    },
    {
        /* ICH10 */
        PCI_VDEVICE(INTEL, 0x3A25),
    },
    {
        /* PCH AHCI */
        PCI_VDEVICE(INTEL, 0x3B22),
    },
    {
        /* PCH AHCI */
        PCI_VDEVICE(INTEL, 0x3B23),
    },
    {
        /* PCH RAID */
        PCI_VDEVICE(INTEL, 0x3B24),
    },
    {
        /* PCH RAID */
        PCI_VDEVICE(INTEL, 0x3B25),
    },
    {
        /* PCH M AHCI */
        PCI_VDEVICE(INTEL, 0x3B29),
    },
    {
        /* PCH RAID */
        PCI_VDEVICE(INTEL, 0x3B2B),
    },
    {
        /* PCH M RAID */
        PCI_VDEVICE(INTEL, 0x3B2C),
    },
    {
        /* PCH AHCI */
        PCI_VDEVICE(INTEL, 0x3B2F),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B0),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B1),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B2),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B3),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B4),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B5),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B6),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19B7),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19BE),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19BF),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C0),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C1),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C2),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C3),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C4),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C5),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C6),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19C7),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19CE),
    },
    {
        /* DNV AHCI */
        PCI_VDEVICE(INTEL, 0x19CF),
    },
    {
        /* CPT AHCI */
        PCI_VDEVICE(INTEL, 0x1C02),
    },
    {
        /* CPT M AHCI */
        PCI_VDEVICE(INTEL, 0x1C03),
    },
    {
        /* CPT RAID */
        PCI_VDEVICE(INTEL, 0x1C04),
    },
    {
        /* CPT M RAID */
        PCI_VDEVICE(INTEL, 0x1C05),
    },
    {
        /* CPT RAID */
        PCI_VDEVICE(INTEL, 0x1C06),
    },
    {
        /* CPT RAID */
        PCI_VDEVICE(INTEL, 0x1C07),
    },
    {
        /* PBG AHCI */
        PCI_VDEVICE(INTEL, 0x1D02),
    },
    {
        /* PBG RAID */
        PCI_VDEVICE(INTEL, 0x1D04),
    },
    {
        /* PBG RAID */
        PCI_VDEVICE(INTEL, 0x1D06),
    },
    {
        /* DH89xxCC AHCI */
        PCI_VDEVICE(INTEL, 0x2323),
    },
    {
        /* Panther Point AHCI */
        PCI_VDEVICE(INTEL, 0x1E02),
    },
    {
        /* Panther M AHCI */
        PCI_VDEVICE(INTEL, 0x1E03),
    },
    {
        /* Panther Point RAID */
        PCI_VDEVICE(INTEL, 0x1E04),
    },
    {
        /* Panther Point RAID */
        PCI_VDEVICE(INTEL, 0x1E05),
    },
    {
        /* Panther Point RAID */
        PCI_VDEVICE(INTEL, 0x1E06),
    },
    {
        /* Panther M RAID */
        PCI_VDEVICE(INTEL, 0x1E07),
    },
    {
        /* Panther Point RAID */
        PCI_VDEVICE(INTEL, 0x1E0E),
    },
    {
        /* Lynx Point AHCI */
        PCI_VDEVICE(INTEL, 0x8C02),
    },
    {
        /* Lynx M AHCI */
        PCI_VDEVICE(INTEL, 0x8C03),
    },
    {
        /* Lynx Point RAID */
        PCI_VDEVICE(INTEL, 0x8C04),
    },
    {
        /* Lynx M RAID */
        PCI_VDEVICE(INTEL, 0x8C05),
    },
    {
        /* Lynx Point RAID */
        PCI_VDEVICE(INTEL, 0x8C06),
    },
    {
        /* Lynx M RAID */
        PCI_VDEVICE(INTEL, 0x8C07),
    },
    {
        /* Lynx Point RAID */
        PCI_VDEVICE(INTEL, 0x8C0E),
    },
    {
        /* Lynx M RAID */
        PCI_VDEVICE(INTEL, 0x8C0F),
    },
    {
        /* Lynx LP AHCI */
        PCI_VDEVICE(INTEL, 0x9C02),
    },
    {
        /* Lynx LP AHCI */
        PCI_VDEVICE(INTEL, 0x9C03),
    },
    {
        /* Lynx LP RAID */
        PCI_VDEVICE(INTEL, 0x9C04),
    },
    {
        /* Lynx LP RAID */
        PCI_VDEVICE(INTEL, 0x9C05),
    },
    {
        /* Lynx LP RAID */
        PCI_VDEVICE(INTEL, 0x9C06),
    },
    {
        /* Lynx LP RAID */
        PCI_VDEVICE(INTEL, 0x9C07),
    },
    {
        /* Lynx LP RAID */
        PCI_VDEVICE(INTEL, 0x9C0E),
    },
    {
        /* Lynx LP RAID */
        PCI_VDEVICE(INTEL, 0x9C0F),
    },
    {
        /* Cannon Lake PCH-LP AHCI */
        PCI_VDEVICE(INTEL, 0x9DD3),
    },
    {
        /* Avoton AHCI */
        PCI_VDEVICE(INTEL, 0x1F22),
    },
    {
        /* Avoton AHCI */
        PCI_VDEVICE(INTEL, 0x1F23),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F24),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F25),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F26),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F27),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F2E),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F2F),
    },
    {
        /* Avoton AHCI */
        PCI_VDEVICE(INTEL, 0x1F32),
    },
    {
        /* Avoton AHCI */
        PCI_VDEVICE(INTEL, 0x1F33),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F34),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F35),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F36),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F37),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F3E),
    },
    {
        /* Avoton RAID */
        PCI_VDEVICE(INTEL, 0x1F3F),
    },
    {
        /* Wellsburg/Lewisburg AHCI*/
        PCI_VDEVICE(INTEL, 0x2823),
    },
    {
        /* *burg SATA0 'RAID' */
        PCI_VDEVICE(INTEL, 0x2826),
    },
    {
        /* *burg SATA1 'RAID' */
        PCI_VDEVICE(INTEL, 0x2827),
    },
    {
        /* *burg SATA2 'RAID' */
        PCI_VDEVICE(INTEL, 0x282F),
    },
    {
        /* Rocket Lake PCH-H RAID */
        PCI_VDEVICE(INTEL, 0x43D4),
    },
    {
        /* Rocket Lake PCH-H RAID */
        PCI_VDEVICE(INTEL, 0x43D5),
    },
    {
        /* Rocket Lake PCH-H RAID */
        PCI_VDEVICE(INTEL, 0x43D6),
    },
    {
        /* Rocket Lake PCH-H RAID */
        PCI_VDEVICE(INTEL, 0x43D7),
    },
    {
        /* Wellsburg AHCI */
        PCI_VDEVICE(INTEL, 0x8D02),
    },
    {
        /* Wellsburg RAID */
        PCI_VDEVICE(INTEL, 0x8D04),
    },
    {
        /* Wellsburg RAID */
        PCI_VDEVICE(INTEL, 0x8D06),
    },
    {
        /* Wellsburg RAID */
        PCI_VDEVICE(INTEL, 0x8D0E),
    },
    {
        /* Wellsburg AHCI */
        PCI_VDEVICE(INTEL, 0x8D62),
    },
    {
        /* Wellsburg RAID */
        PCI_VDEVICE(INTEL, 0x8D64),
    },
    {
        /* Wellsburg RAID */
        PCI_VDEVICE(INTEL, 0x8D66),
    },
    {
        /* Wellsburg RAID */
        PCI_VDEVICE(INTEL, 0x8D6E),
    },
    {
        /* Coleto Creek AHCI */
        PCI_VDEVICE(INTEL, 0x23A3),
    },
    {
        /* Wildcat LP AHCI */
        PCI_VDEVICE(INTEL, 0x9C83),
    },
    {
        /* Wildcat LP RAID */
        PCI_VDEVICE(INTEL, 0x9C85),
    },
    {
        /* Wildcat LP RAID */
        PCI_VDEVICE(INTEL, 0x9C87),
    },
    {
        /* Wildcat LP RAID */
        PCI_VDEVICE(INTEL, 0x9C8F),
    },
    {
        /* 9 Series AHCI */
        PCI_VDEVICE(INTEL, 0x8C82),
    },
    {
        /* 9 Series M AHCI */
        PCI_VDEVICE(INTEL, 0x8C83),
    },
    {
        /* 9 Series RAID */
        PCI_VDEVICE(INTEL, 0x8C84),
    },
    {
        /* 9 Series M RAID */
        PCI_VDEVICE(INTEL, 0x8C85),
    },
    {
        /* 9 Series RAID */
        PCI_VDEVICE(INTEL, 0x8C86),
    },
    {
        /* 9 Series M RAID */
        PCI_VDEVICE(INTEL, 0x8C87),
    },
    {
        /* 9 Series RAID */
        PCI_VDEVICE(INTEL, 0x8C8E),
    },
    {
        /* 9 Series M RAID */
        PCI_VDEVICE(INTEL, 0x8C8F),
    },
    {
        /* Sunrise LP AHCI */
        PCI_VDEVICE(INTEL, 0x9D03),
    },
    {
        /* Sunrise LP RAID */
        PCI_VDEVICE(INTEL, 0x9D05),
    },
    {
        /* Sunrise LP RAID */
        PCI_VDEVICE(INTEL, 0x9D07),
    },
    {
        /* Sunrise Point-H AHCI */
        PCI_VDEVICE(INTEL, 0xA102),
    },
    {
        /* Sunrise M AHCI */
        PCI_VDEVICE(INTEL, 0xA103),
    },
    {
        /* Sunrise Point-H RAID */
        PCI_VDEVICE(INTEL, 0xA105),
    },
    {
        /* Sunrise Point-H RAID */
        PCI_VDEVICE(INTEL, 0xA106),
    },
    {
        /* Sunrise M RAID */
        PCI_VDEVICE(INTEL, 0xA107),
    },
    {
        /* Sunrise Point-H RAID */
        PCI_VDEVICE(INTEL, 0xA10F),
    },
    {
        /* Lewisburg AHCI*/
        PCI_VDEVICE(INTEL, 0xA182),
    },
    {
        /* Lewisburg RAID*/
        PCI_VDEVICE(INTEL, 0xA186),
    },
    {
        /* Lewisburg RAID*/
        PCI_VDEVICE(INTEL, 0xA1D2),
    },
    {
        /* Lewisburg RAID*/
        PCI_VDEVICE(INTEL, 0xA1D6),
    },
    {
        /* Lewisburg AHCI*/
        PCI_VDEVICE(INTEL, 0xA202),
    },
    {
        /* Lewisburg RAID*/
        PCI_VDEVICE(INTEL, 0xA206),
    },
    {
        /* Lewisburg RAID*/
        PCI_VDEVICE(INTEL, 0xA252),
    },
    {
        /* Lewisburg RAID*/
        PCI_VDEVICE(INTEL, 0xA256),
    },
    {
        /* Cannon Lake PCH-H RAID */
        PCI_VDEVICE(INTEL, 0xA356),
    },
    {
        /* Comet Lake-H RAID */
        PCI_VDEVICE(INTEL, 0x06D7),
    },
    {
        /* Comet Lake PCH-V RAID */
        PCI_VDEVICE(INTEL, 0xA386),
    },
    {
        /* Bay Trail AHCI */
        PCI_VDEVICE(INTEL, 0x0F22),
    },
    {
        /* Bay Trail AHCI */
        PCI_VDEVICE(INTEL, 0x0F23),
    },
    {
        /* Cherry Tr. AHCI */
        PCI_VDEVICE(INTEL, 0x22A3),
    },
    {
        /* ApolloLake AHCI */
        PCI_VDEVICE(INTEL, 0x5AE3),
    },
    {
        /* Ice Lake LP AHCI */
        PCI_VDEVICE(INTEL, 0x34D3),
    },
    {
        /* Comet Lake PCH-U AHCI */
        PCI_VDEVICE(INTEL, 0x02D3),
    },
    {
        /* Comet Lake PCH RAID */
        PCI_VDEVICE(INTEL, 0x02D7),
    },

    /* Elkhart Lake IDs 0x4b60 & 0x4b62 https://sata-io.org/product/8803 not tested yet */
    {
        /* Elkhart Lake AHCI */
        PCI_VDEVICE(INTEL, 0x4B63),
    },
    {
        /* JMicron JMB582/585: force 32-bit DMA (broken 64-bit implementation) */
        PCI_VDEVICE(JMICRON, 0x0582),

    },
    {
        PCI_VDEVICE(JMICRON, 0x0585),
    },
    {
        /* JMicron 360/1/3/5/6, match class to avoid IDE function */
        PCI_DEVICE(PCI_VENDOR_ID_JMICRON, PCI_ANY_ID),
        .class = PCI_CLASS_STORAGE_SATA_AHCI,
        .class_mask = 0xFFFFFF,

    },
    /* JMicron 362B and 362C have an AHCI function with IDE class code */
    {
        PCI_VDEVICE(JMICRON, 0x2362),

    },
    {
        PCI_VDEVICE(JMICRON, 0x236F),

    },
    /* May need to update quirk_jmicron_async_suspend() for additions */

    /* ATI */
    {
        /* ATI SB600 */
        PCI_VDEVICE(ATI, 0x4380),
    },
    {
        /* ATI SB700/800 */
        PCI_VDEVICE(ATI, 0x4390),
    },
    {
        /* ATI SB700/800 */
        PCI_VDEVICE(ATI, 0x4391),
    },
    {
        /* ATI SB700/800 */
        PCI_VDEVICE(ATI, 0x4392),
    },
    {
        /* ATI SB700/800 */
        PCI_VDEVICE(ATI, 0x4393),
    },
    {
        /* ATI SB700/800 */
        PCI_VDEVICE(ATI, 0x4394),
    },
    {
        /* ATI SB700/800 */
        PCI_VDEVICE(ATI, 0x4395),
    },

    /* Amazon's Annapurna Labs support */
    {
        PCI_DEVICE(PCI_VENDOR_ID_AMAZON_ANNAPURNA_LABS, 0x0031),
        .class = PCI_CLASS_STORAGE_SATA_AHCI,
        .class_mask = 0xFFFFFF,

    },
    /* AMD */
    {
        /* AMD Hudson-2 */
        PCI_VDEVICE(AMD, 0x7800),
    },
    {
        /* AMD Hudson-2 (AHCI mode) */
        PCI_VDEVICE(AMD, 0x7801),
    },
    {
        /* AMD CZ */
        PCI_VDEVICE(AMD, 0x7900),
    },
    {
        /* AMD Green Sardine */
        PCI_VDEVICE(AMD, 0x7901),
    },
    /* AMD is using RAID class only for ahci controllers */
    {
        PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_ANY_ID),
        .class = PCI_CLASS_STORAGE_RAID << 8,
        .class_mask = 0xFFFFFF,
    },

    /* Dell S140/S150 */
    {
        PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_ANY_ID),
        .class = PCI_CLASS_STORAGE_RAID << 8,
        .class_mask = 0xFFFFFF,

    },

    /* VIA */
    {
        /* VIA VT8251 */
        PCI_VDEVICE(VIA, 0x3349),
    },
    {
        /* VIA VT8251 */
        PCI_VDEVICE(VIA, 0x6287),
    },

    /* NVIDIA */
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x044C),
    },
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x044D),
    },
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x044E),
    },
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x044F),
    },
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x045C),
    },
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x045D),
    },
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x045E),
    },
    {
        /* MCP65 */
        PCI_VDEVICE(NVIDIA, 0x045F),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0550),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0551),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0552),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0553),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0554),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0555),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0556),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0557),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0558),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x0559),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x055A),
    },
    {
        /* MCP67 */
        PCI_VDEVICE(NVIDIA, 0x055B),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0580),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0581),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0582),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0583),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0584),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0585),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0586),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0587),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0588),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x0589),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x058A),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x058B),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x058C),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x058D),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x058E),
    },
    {
        /* Linux ID */
        PCI_VDEVICE(NVIDIA, 0x058F),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F0),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F1),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F2),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F3),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F4),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F5),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F6),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F7),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F8),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07F9),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07FA),
    },
    {
        /* MCP73 */
        PCI_VDEVICE(NVIDIA, 0x07FB),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD0),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD1),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD2),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD3),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD4),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD5),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD6),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD7),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD8),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0AD9),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0ADA),
    },
    {
        /* MCP77 */
        PCI_VDEVICE(NVIDIA, 0x0ADB),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0AB4),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0AB5),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0AB6),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0AB7),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0AB8),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0AB9),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0ABA),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0ABB),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0ABC),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0ABD),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0ABE),
    },
    {
        /* MCP79 */
        PCI_VDEVICE(NVIDIA, 0x0ABF),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D84),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D85),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D86),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D87),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D88),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D89),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D8A),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D8B),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D8C),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D8D),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D8E),
    },
    {
        /* MCP89 */
        PCI_VDEVICE(NVIDIA, 0x0D8F),
    },

    /* SiS */
    {
        /* SiS 966 */
        PCI_VDEVICE(SI, 0x1184),
    },
    {
        /* SiS 968 */
        PCI_VDEVICE(SI, 0x1185),
    },
    {
        /* SiS 968 */
        PCI_VDEVICE(SI, 0x0186),
    },

    /* ST Microelectronics */
    {
        /* ST ConneXt */
        PCI_VDEVICE(STMICRO, 0xCC06),
    },

    /* Marvell */
    {
        /* 6145 */
        PCI_VDEVICE(MARVELL, 0x6145),
    },
    {
        /* 6121 */
        PCI_VDEVICE(MARVELL, 0x6121),
    },
    {
        /* 88se9128 */
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9123),
        .class = PCI_CLASS_STORAGE_SATA_AHCI,
        .class_mask = 0xFFFFFF,
    },
    {
        /* 88se9125 */
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9125),
    },
    {
        /* 88se9170 */
        PCI_DEVICE_SUB(PCI_VENDOR_ID_MARVELL_EXT, 0x9178,
                       PCI_VENDOR_ID_MARVELL_EXT, 0x9170),
    },
    {
        /* 88se9172 */
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x917A),
    },
    {
        /* 88se9182 */
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9172),
    },
    {
        /* 88se9172 */
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9182),
    },
    {
        /* 88se9172 on some Gigabyte */
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9192),
    },
    {
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x91A0),
    },
    {
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x91A2), /* 88se91a2 */
    },
    {
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x91A3),
    },
    {
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9215),
    },
    {
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9230),
    },
    {
        PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9235),
    },

    /* TTI */
    {
        /* highpoint rocketraid 642L */
        PCI_DEVICE(PCI_VENDOR_ID_TTI, 0x0642),
    },
    {
        /* highpoint rocketraid 644L */
        PCI_DEVICE(PCI_VENDOR_ID_TTI, 0x0645),
    },

    /* Promise */
    {
        /* PDC42819 */
        PCI_VDEVICE(PROMISE, 0x3F20),
    },
    {
        /* FastTrak TX8660 ahci-mode */
        PCI_VDEVICE(PROMISE, 0x3781),
    },

    /* ASMedia */
    {
        /* ASM1060 */
        PCI_VDEVICE(ASMEDIA, 0x0601),
    },
    {
        /* ASM1060 */
        PCI_VDEVICE(ASMEDIA, 0x0602),
    },
    {
        /* ASM1061 */
        PCI_VDEVICE(ASMEDIA, 0x0611),
    },
    {
        /* ASM1061/1062 */
        PCI_VDEVICE(ASMEDIA, 0x0612),
    },
    {
        /* ASM1061R */
        PCI_VDEVICE(ASMEDIA, 0x0621),
    },
    {
        /* ASM1062R */
        PCI_VDEVICE(ASMEDIA, 0x0622),
    },
    {
        /* ASM1062+JMB575 */
        PCI_VDEVICE(ASMEDIA, 0x0624),
    },
    {
        /* ASM1062A */
        PCI_VDEVICE(ASMEDIA, 0x1062),
    },
    {
        /* ASM1064 */
        PCI_VDEVICE(ASMEDIA, 0x1064),
    },
    {
        /* ASM1164 */
        PCI_VDEVICE(ASMEDIA, 0x1164),
    },
    {
        /* ASM1165 */
        PCI_VDEVICE(ASMEDIA, 0x1165),
    },
    {
        /* ASM1166 */
        PCI_VDEVICE(ASMEDIA, 0x1166),
    },
    {
        /*
         * Samsung SSDs found on some macbooks.  NCQ times out if MSI is
         * enabled.  https://bugzilla.kernel.org/show_bug.cgi?id=60731
         */
        PCI_VDEVICE(SAMSUNG, 0x1600),

    },
    {
        PCI_VDEVICE(SAMSUNG, 0xA800),
    },
    {
        /* Enmotus */
        PCI_DEVICE(0x1C44, 0x8000),
    },
    {
        /* Loongson */
        PCI_VDEVICE(LOONGSON, 0x7A08),

    },
    {
        /* Generic, PCI class code for AHCI */
        PCI_DEVICE_CLASS(PCI_CLASS_STORAGE_SATA_AHCI, 0xFFFFFF),
    },

    {} /* terminate list */
};

static pci_driver_t ahci_pci_driver = {
    .name = DRV_NAME,
    .id_table = ahci_pci_tbl,
    .probe = ahci_init_one,
    .remove = ahci_remove_one,
};

MODULE_PCI_DRIVER(ahci_pci_driver)

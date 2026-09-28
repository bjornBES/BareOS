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

#include "drivers/driver.h"
#include "pci/pci.h"

#include "kerrno.h"

#include "string.h"

#define MODULE            "ahci-driver"

#define DRV_NAME          "ahci"

#define DEFAULT_BAR_INDEX 5

status_t ahci_init_one(pci_device_t *dev, pci_device_id_t *id)
{
    ENTER_FUNC("%p, %p", dev, id);

    status_t ret = 0;

    ret = pci_set_command_flags(dev, PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
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
        KERRNO_RETURN(KERRNO_DONT_DO_THAT, "bar%u is I/O", bar_index);
    }

    ahci_mem_t *abar = NULL;
    ret = pci_map_bar(dev, bar_index, abar);
    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }

    dev->dev.class = DEVICE_BLOCK;
    dev->dev.init_name = "AHCI";

    return KERRNO_SUCCESSES;
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

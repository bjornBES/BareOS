/*
 * File: 8250_pci.c
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/pci/pci_id.h"
#include "drivers/pci/pci_ids.h"
#include "drivers/pci/pci.h"
#include "drivers/base/drivert.h"
#include "debug/debug.h"
#include "module.h"

#define MODULE "8250-pci"

#define PCI_VENDOR_ID_SBSMODULARIO           0x124B
#define PCI_SUBVENDOR_ID_SBSMODULARIO        0x124B
#define PCI_DEVICE_ID_OCTPRO                 0x0001
#define PCI_SUBDEVICE_ID_OCTPRO232           0x0108
#define PCI_SUBDEVICE_ID_OCTPRO422           0x0208
#define PCI_SUBDEVICE_ID_POCTAL232           0x0308
#define PCI_SUBDEVICE_ID_POCTAL422           0x0408
#define PCI_SUBDEVICE_ID_SIIG_DUAL_00        0x2500
#define PCI_SUBDEVICE_ID_SIIG_DUAL_30        0x2530
#define PCI_DEVICE_ID_INTEL_CE4100_UART      0x2E66
#define PCI_DEVICE_ID_ADVANTECH_PCI1600      0x1600
#define PCI_DEVICE_ID_ADVANTECH_PCI1600_1611 0x1611
#define PCI_DEVICE_ID_ADVANTECH_PCI3620      0x3620
#define PCI_DEVICE_ID_ADVANTECH_PCI3618      0x3618
#define PCI_DEVICE_ID_ADVANTECH_PCIf618      0xF618
#define PCI_DEVICE_ID_TITAN_200I             0x8028
#define PCI_DEVICE_ID_TITAN_400I             0x8048
#define PCI_DEVICE_ID_TITAN_800I             0x8088
#define PCI_DEVICE_ID_TITAN_800EH            0xA007
#define PCI_DEVICE_ID_TITAN_800EHB           0xA008
#define PCI_DEVICE_ID_TITAN_400EH            0xA009
#define PCI_DEVICE_ID_TITAN_100E             0xA010
#define PCI_DEVICE_ID_TITAN_200E             0xA012
#define PCI_DEVICE_ID_TITAN_400E             0xA013
#define PCI_DEVICE_ID_TITAN_800E             0xA014
#define PCI_DEVICE_ID_TITAN_200EI            0xA016
#define PCI_DEVICE_ID_TITAN_200EISI          0xA017
#define PCI_DEVICE_ID_TITAN_200V3            0xA306
#define PCI_DEVICE_ID_TITAN_400V3            0xA310
#define PCI_DEVICE_ID_TITAN_410V3            0xA312
#define PCI_DEVICE_ID_TITAN_800V3            0xA314
#define PCI_DEVICE_ID_TITAN_800V3B           0xA315
#define PCI_DEVICE_ID_OXSEMI_16PCI958        0x9538
#define PCIE_DEVICE_ID_NEO_2_OX_IBM          0x00F6
#define PCI_DEVICE_ID_PLX_CRONYX_OMEGA       0xC001
#define PCI_DEVICE_ID_INTEL_PATSBURG_KT      0x1D3D

#define PCI_DEVICE_ID_WCHCN_CH352_2S         0x3253
#define PCI_DEVICE_ID_WCHCN_CH355_4S         0x7173

#define PCI_VENDOR_ID_AGESTAR                0x5372
#define PCI_DEVICE_ID_AGESTAR_9375           0x6872
#define PCI_DEVICE_ID_BROADCOM_TRUMANAGE     0x160A
#define PCI_DEVICE_ID_AMCC_ADDIDATA_APCI7800 0x818E

#define PCI_DEVICE_ID_WCHIC_CH384_4S         0x3470
#define PCI_DEVICE_ID_WCHIC_CH384_8S         0x3853

#define PCI_DEVICE_ID_ADDIDATA_CPCI7500      0x7003
#define PCI_DEVICE_ID_ADDIDATA_CPCI7500_NG   0x7024
#define PCI_DEVICE_ID_ADDIDATA_CPCI7420_NG   0x7025
#define PCI_DEVICE_ID_ADDIDATA_CPCI7300_NG   0x7026

#define PCI_VENDOR_ID_SYSTEMBASE             0x14A1

/* Unknown vendors/cards - this should not be in linux/pci_ids.h */
#define PCI_SUBDEVICE_ID_UNKNOWN_0x1584      0x1584
#define PCI_SUBDEVICE_ID_UNKNOWN_0x1588      0x1588

/*
 * Oxford Semiconductor Inc.
 * Check if an OxSemi device is part of the Tornado range of devices.
 */
#define PCI_VENDOR_ID_ENDRUN                 0x7401
#define PCI_DEVICE_ID_ENDRUN_1588            0xE100

#define PCI_DEVICE_ID_HPE_PCI_SERIAL         0x37E
#define PCIE_VENDOR_ID_ASIX                  0x125B
#define PCIE_DEVICE_ID_AX99100               0x9100

enum pci_board_num_t
{
    pbn_default = 0,

    pbn_b0_1_115200,
    pbn_b0_2_115200,
    pbn_b0_4_115200,
    pbn_b0_5_115200,
    pbn_b0_8_115200,

    pbn_b0_1_921600,
    pbn_b0_2_921600,
    pbn_b0_4_921600,
    pbn_b0_8_921600,

    pbn_b0_2_1130000,

    pbn_b0_4_1152000,

    pbn_b0_4_1250000,

    pbn_b0_2_1843200,
    pbn_b0_4_1843200,

    pbn_b0_1_15625000,

    pbn_b0_bt_1_115200,
    pbn_b0_bt_2_115200,
    pbn_b0_bt_4_115200,
    pbn_b0_bt_8_115200,

    pbn_b0_bt_1_460800,
    pbn_b0_bt_2_460800,
    pbn_b0_bt_4_460800,

    pbn_b0_bt_1_921600,
    pbn_b0_bt_2_921600,
    pbn_b0_bt_4_921600,
    pbn_b0_bt_8_921600,

    pbn_b1_1_115200,
    pbn_b1_2_115200,
    pbn_b1_4_115200,
    pbn_b1_8_115200,
    pbn_b1_16_115200,

    pbn_b1_1_921600,
    pbn_b1_2_921600,
    pbn_b1_4_921600,
    pbn_b1_8_921600,

    pbn_b1_2_1250000,

    pbn_b1_bt_1_115200,
    pbn_b1_bt_2_115200,
    pbn_b1_bt_4_115200,

    pbn_b1_bt_2_921600,

    pbn_b1_1_1382400,
    pbn_b1_2_1382400,
    pbn_b1_4_1382400,
    pbn_b1_8_1382400,

    pbn_b2_1_115200,
    pbn_b2_2_115200,
    pbn_b2_4_115200,
    pbn_b2_8_115200,

    pbn_b2_1_460800,
    pbn_b2_4_460800,
    pbn_b2_8_460800,
    pbn_b2_16_460800,

    pbn_b2_1_921600,
    pbn_b2_4_921600,
    pbn_b2_8_921600,

    pbn_b2_8_1152000,

    pbn_b2_bt_1_115200,
    pbn_b2_bt_2_115200,
    pbn_b2_bt_4_115200,

    pbn_b2_bt_2_921600,
    pbn_b2_bt_4_921600,

    pbn_b3_2_115200,
    pbn_b3_4_115200,
    pbn_b3_8_115200,

    pbn_b4_bt_2_921600,
    pbn_b4_bt_4_921600,
    pbn_b4_bt_8_921600,

    /*
     * Board-specific versions.
     */
    pbn_panacom,
    pbn_panacom2,
    pbn_panacom4,
    pbn_plx_romulus,
    pbn_oxsemi,
    pbn_oxsemi_1_15625000,
    pbn_oxsemi_2_15625000,
    pbn_oxsemi_4_15625000,
    pbn_oxsemi_8_15625000,
    pbn_intel_i960,
    pbn_sgi_ioc3,
    pbn_computone_4,
    pbn_computone_6,
    pbn_computone_8,
    pbn_sbsxrsio,
    pbn_pasemi_1682M,
    pbn_ni8430_2,
    pbn_ni8430_4,
    pbn_ni8430_8,
    pbn_ni8430_16,
    pbn_ADDIDATA_PCIe_1_3906250,
    pbn_ADDIDATA_PCIe_2_3906250,
    pbn_ADDIDATA_PCIe_4_3906250,
    pbn_ADDIDATA_PCIe_8_3906250,
    pbn_ce4100_1_115200,
    pbn_omegapci,
    pbn_NETMOS9900_2s_115200,
    pbn_brcm_trumanage,
    pbn_fintek_4,
    pbn_fintek_8,
    pbn_fintek_12,
    pbn_fintek_F81504A,
    pbn_fintek_F81508A,
    pbn_fintek_F81512A,
    pbn_wch382_2,
    pbn_wch384_4,
    pbn_wch384_8,
    pbn_sunix_pci_1s,
    pbn_sunix_pci_2s,
    pbn_sunix_pci_4s,
    pbn_sunix_pci_8s,
    pbn_sunix_pci_16s,
    pbn_titan_1_4000000,
    pbn_titan_2_4000000,
    pbn_titan_4_4000000,
    pbn_titan_8_4000000,
};

status_t pciserial_init_one(pci_device_t *pci_dev, pci_device_id_t *id)
{
    ENTER_FUNC("%p, %p", pci_dev, id);
    return 1;
}

void pciserial_remove_one(pci_device_t *pci_dev)
{
}

static pci_device_id_t serial_pci_tbl[] = {
    {
        PCI_VDEVICE_SUB(ADVANTECH, PCI_DEVICE_ID_ADVANTECH_PCI1600,
                        PCI_DEVICE_ID_ADVANTECH_PCI1600_1611, PCI_ANY_ID),
        .driver_data = pbn_b0_4_921600,
    },
    {
        /* Advantech use PCI_DEVICE_ID_ADVANTECH_PCI3620 (0x3620) as 'PCI_SUBVENDOR_ID' */
        PCI_VDEVICE_SUB(ADVANTECH, PCI_DEVICE_ID_ADVANTECH_PCI3620,
                        PCI_DEVICE_ID_ADVANTECH_PCI3620, 0x0001),
        .driver_data = pbn_b2_8_921600,
    },
    {
        /* Advantech also use 0x3618 and 0xf618 */
        PCI_VDEVICE_SUB(ADVANTECH, PCI_DEVICE_ID_ADVANTECH_PCI3618,
                        PCI_DEVICE_ID_ADVANTECH_PCI3618, PCI_ANY_ID),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE_SUB(ADVANTECH, PCI_DEVICE_ID_ADVANTECH_PCIf618,
                        PCI_DEVICE_ID_ADVANTECH_PCI3618, PCI_ANY_ID),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V960,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH8_232),
        .driver_data = pbn_b1_8_1382400,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V960,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH4_232),
        .driver_data = pbn_b1_4_1382400,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V960,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH2_232),
        .driver_data = pbn_b1_2_1382400,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH8_232),
        .driver_data = pbn_b1_8_1382400,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH4_232),
        .driver_data = pbn_b1_4_1382400,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH2_232),
        .driver_data = pbn_b1_2_1382400,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH8_485),
        .driver_data = pbn_b1_8_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH8_485_4_4),
        .driver_data = pbn_b1_8_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH4_485),
        .driver_data = pbn_b1_4_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH4_485_2_2),
        .driver_data = pbn_b1_4_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH2_485),
        .driver_data = pbn_b1_2_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH8_485_2_6),
        .driver_data = pbn_b1_8_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH081101V1),
        .driver_data = pbn_b1_8_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH041101V1),
        .driver_data = pbn_b1_4_921600,
    },
    {
        PCI_VDEVICE_SUB(V3, PCI_DEVICE_ID_V3_V351,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_BH2_20MHZ),
        .driver_data = pbn_b1_2_1250000,
    },
    {
        PCI_VDEVICE_SUB(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI954,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_TITAN_2),
        .driver_data = pbn_b0_2_1843200,
    },
    {
        PCI_VDEVICE_SUB(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI954,
                        PCI_SUBVENDOR_ID_CONNECT_TECH, PCI_SUBDEVICE_ID_CONNECT_TECH_TITAN_4),
        .driver_data = pbn_b0_4_1843200,
    },
    {
        PCI_VDEVICE_SUB(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI954,
                        PCI_VENDOR_ID_AFAVLAB, PCI_SUBDEVICE_ID_AFAVLAB_P061),
        .driver_data = pbn_b0_4_1152000,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_U530),
        .driver_data = pbn_b2_bt_1_115200,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_UCOMM2),
        .driver_data = pbn_b2_bt_2_115200,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_UCOMM422),
        .driver_data = pbn_b2_bt_4_115200,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_UCOMM232),
        .driver_data = pbn_b2_bt_2_115200,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_COMM4),
        .driver_data = pbn_b2_bt_4_115200,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_COMM8),
        .driver_data = pbn_b2_8_115200,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_7803),
        .driver_data = pbn_b2_8_460800,
    },
    {
        PCI_VDEVICE(SEALEVEL, PCI_DEVICE_ID_SEALEVEL_UCOMM8),
        .driver_data = pbn_b2_8_115200,
    },
    {
        PCI_VDEVICE(PLX, PCI_DEVICE_ID_PLX_GTEK_SERIAL2),
        .driver_data = pbn_b2_bt_2_115200,
    },
    {
        PCI_VDEVICE(PLX, PCI_DEVICE_ID_PLX_SPCOM200),
        .driver_data = pbn_b2_bt_2_921600,
    },
    {
        /* VScom SPCOM800, from sl@s.pl */
        PCI_VDEVICE(PLX, PCI_DEVICE_ID_PLX_SPCOM800),
        .driver_data = pbn_b2_8_921600,
    },
    {
        PCI_VDEVICE(PLX, PCI_DEVICE_ID_PLX_1077),
        .driver_data = pbn_b2_4_921600,
    },
    {
        /* Unknown card - subdevice 0x1584 */
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_VENDOR_ID_PLX, PCI_SUBDEVICE_ID_UNKNOWN_0x1584),
        .driver_data = pbn_b2_4_115200,
    },
    {
        /* Unknown card - subdevice 0x1588 */
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_VENDOR_ID_PLX, PCI_SUBDEVICE_ID_UNKNOWN_0x1588),
        .driver_data = pbn_b2_8_115200,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_KEYSPAN, PCI_SUBDEVICE_ID_KEYSPAN_SX2),
        .driver_data = pbn_panacom,
    },
    {
        PCI_VDEVICE(PANACOM, PCI_DEVICE_ID_PANACOM_QUADMODEM),
        .driver_data = pbn_panacom4,
    },
    {
        PCI_VDEVICE(PANACOM, PCI_DEVICE_ID_PANACOM_DUALMODEM),
        .driver_data = pbn_panacom2,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9030,
                        PCI_VENDOR_ID_ESDGMBH, PCI_DEVICE_ID_ESDGMBH_CPCIASIO4),
        .driver_data = pbn_b2_4_115200,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_CHASE_PCIFAST, PCI_SUBDEVICE_ID_CHASE_PCIFAST4),
        .driver_data = pbn_b2_4_460800,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_CHASE_PCIFAST, PCI_SUBDEVICE_ID_CHASE_PCIFAST8),
        .driver_data = pbn_b2_8_460800,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_CHASE_PCIFAST, PCI_SUBDEVICE_ID_CHASE_PCIFAST16),
        .driver_data = pbn_b2_16_460800,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_CHASE_PCIFAST, PCI_SUBDEVICE_ID_CHASE_PCIFAST16FMC),
        .driver_data = pbn_b2_16_460800,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_CHASE_PCIRAS, PCI_SUBDEVICE_ID_CHASE_PCIRAS4),
        .driver_data = pbn_b2_4_460800,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_CHASE_PCIRAS, PCI_SUBDEVICE_ID_CHASE_PCIRAS8),
        .driver_data = pbn_b2_8_460800,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9050,
                        PCI_SUBVENDOR_ID_EXSYS, PCI_SUBDEVICE_ID_EXSYS_4055),
        .driver_data = pbn_b2_4_115200,
    },
    {
        /*
         * Megawolf Romulus PCI Serial Card, from Mike Hudson
         * (Exoray@isys.ca)
         */
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_ROMULUS,
                        0x10B5, 0x106A),
        .driver_data = pbn_plx_romulus,
    },

    /*
     * Quatech cards. These actually have configurable clocks but for
     * now we just use the default.
     *
     * 100 series are RS232, 200 series RS422,
     */
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_QSC100),
        .driver_data = pbn_b1_4_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSC100),
        .driver_data = pbn_b1_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSC100E),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSC200),
        .driver_data = pbn_b1_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSC200E),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_QSC200),
        .driver_data = pbn_b1_4_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_ESC100D),
        .driver_data = pbn_b1_8_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_ESC100M),
        .driver_data = pbn_b1_8_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_QSCP100),
        .driver_data = pbn_b1_4_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSCP100),
        .driver_data = pbn_b1_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_QSCP200),
        .driver_data = pbn_b1_4_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSCP200),
        .driver_data = pbn_b1_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_QSCLP100),
        .driver_data = pbn_b2_4_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSCLP100),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_SSCLP100),
        .driver_data = pbn_b2_1_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_QSCLP200),
        .driver_data = pbn_b2_4_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_DSCLP200),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_SSCLP200),
        .driver_data = pbn_b2_1_115200,
    },
    {
        PCI_VDEVICE(QUATECH, PCI_DEVICE_ID_QUATECH_ESCLP100),
        .driver_data = pbn_b0_8_115200,
    },
    {
        PCI_VDEVICE_SUB(SPECIALIX, PCI_DEVICE_ID_OXSEMI_16PCI954,
                        PCI_VENDOR_ID_SPECIALIX, PCI_SUBDEVICE_ID_SPECIALIX_SPEED4),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE_SUB(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI954,
                        PCI_SUBVENDOR_ID_SIIG, PCI_SUBDEVICE_ID_SIIG_QUARTET_SERIAL),
        .driver_data = pbn_b0_4_1152000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0x9505),
        .driver_data = pbn_b0_bt_2_921600,
    },
    {
        /*
         * The below card is a little controversial since it is the
         * subject of a PCI vendor/device ID clash.  (See
         * www.ussg.iu.edu/hypermail/linux/kernel/0303.1/0516.html).
         * For now just used the hex ID 0x950a.
         */
        PCI_VDEVICE_SUB(OXSEMI, 0x950A,
                        PCI_SUBVENDOR_ID_SIIG, PCI_SUBDEVICE_ID_SIIG_DUAL_00),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE_SUB(OXSEMI, 0x950A,
                        PCI_SUBVENDOR_ID_SIIG, PCI_SUBDEVICE_ID_SIIG_DUAL_30),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(OXSEMI, 0x950A),
        .driver_data = pbn_b0_2_1130000,
    },
    {
        PCI_VDEVICE_SUB(OXSEMI, PCI_DEVICE_ID_OXSEMI_C950,
                        PCI_VENDOR_ID_OXSEMI, PCI_SUBDEVICE_ID_OXSEMI_C950),
        .driver_data = pbn_b0_1_921600,
    },
    {
        PCI_VDEVICE(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI954),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI952),
        .driver_data = pbn_b0_bt_2_921600,
    },
    {
        PCI_VDEVICE(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI958),
        .driver_data = pbn_b2_8_1152000,
    },

    /*
     * Oxford Semiconductor Inc. Tornado PCI express device range.
     */
    {
        PCI_VDEVICE(OXSEMI, 0xC101), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC105), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC11B), /* OXPCIe952 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC11F), /* OXPCIe952 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC120), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC124), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC138), /* OXPCIe952 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC13D), /* OXPCIe952 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC140), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC141), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC144), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC145), /* OXPCIe952 1 Legacy UART */
        .driver_data = pbn_b0_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC158), /* OXPCIe952 2 Native UART */
        .driver_data = pbn_oxsemi_2_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC15D), /* OXPCIe952 2 Native UART */
        .driver_data = pbn_oxsemi_2_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC208), /* OXPCIe954 4 Native UART */
        .driver_data = pbn_oxsemi_4_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC20D), /* OXPCIe954 4 Native UART */
        .driver_data = pbn_oxsemi_4_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC308), /* OXPCIe958 8 Native UART */
        .driver_data = pbn_oxsemi_8_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC30D), /* OXPCIe958 8 Native UART */
        .driver_data = pbn_oxsemi_8_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC40B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC40F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC41B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC41F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC42B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC42F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC43B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC43F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC44B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC44F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC45B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC45F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC46B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC46F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC47B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC47F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC48B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC48F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC49B), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC49F), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC4AB), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC4AF), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC4BB), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC4BF), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC4CB), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        PCI_VDEVICE(OXSEMI, 0xC4CF), /* OXPCIe200 1 Native UART */
        .driver_data = pbn_oxsemi_1_15625000,
    },
    /*
     * Mainpine Inc. IQ Express "Rev3" utilizing OxSemi Tornado
     */
    {
        /* IQ Express 1 Port V.34 Super-G3 Fax */
        PCI_VDEVICE_SUB(MAINPINE, 0x4000,
                        PCI_VENDOR_ID_MAINPINE, 0x4001),
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        /* IQ Express 2 Port V.34 Super-G3 Fax */
        PCI_VDEVICE_SUB(MAINPINE, 0x4000,
                        PCI_VENDOR_ID_MAINPINE, 0x4002),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    {
        /* IQ Express 4 Port V.34 Super-G3 Fax */
        PCI_VDEVICE_SUB(MAINPINE, 0x4000,
                        PCI_VENDOR_ID_MAINPINE, 0x4004),
        .driver_data = pbn_oxsemi_4_15625000,
    },
    {
        /* IQ Express 8 Port V.34 Super-G3 Fax */
        PCI_VDEVICE_SUB(MAINPINE, 0x4000,
                        PCI_VENDOR_ID_MAINPINE, 0x4008),
        .driver_data = pbn_oxsemi_8_15625000,
    },

    /*
     * Digi/IBM PCIe 2-port Async EIA-232 Adapter utilizing OxSemi Tornado
     */
    {
        PCI_VDEVICE_SUB(DIGI, PCIE_DEVICE_ID_NEO_2_OX_IBM,
                        PCI_SUBVENDOR_ID_IBM, PCI_ANY_ID),
        .driver_data = pbn_oxsemi_2_15625000,
    },

    /*
     * EndRun Technologies. PCI express device range.
     * EndRun PTP/1588 has 2 Native UARTs utilizing OxSemi 952.
     */
    {
        PCI_VDEVICE(ENDRUN, PCI_DEVICE_ID_ENDRUN_1588),
        .driver_data = pbn_oxsemi_2_15625000,
    },

    /*
     * SBS Technologies, Inc. P-Octal and PMC-OCTPRO cards,
     * from skokodyn@yahoo.com
     */
    {
        PCI_VDEVICE_SUB(SBSMODULARIO, PCI_DEVICE_ID_OCTPRO,
                        PCI_SUBVENDOR_ID_SBSMODULARIO, PCI_SUBDEVICE_ID_OCTPRO232),
        .driver_data = pbn_sbsxrsio,
    },
    {
        PCI_VDEVICE_SUB(SBSMODULARIO, PCI_DEVICE_ID_OCTPRO,
                        PCI_SUBVENDOR_ID_SBSMODULARIO, PCI_SUBDEVICE_ID_OCTPRO422),
        .driver_data = pbn_sbsxrsio,
    },
    {
        PCI_VDEVICE_SUB(SBSMODULARIO, PCI_DEVICE_ID_OCTPRO,
                        PCI_SUBVENDOR_ID_SBSMODULARIO, PCI_SUBDEVICE_ID_POCTAL232),
        .driver_data = pbn_sbsxrsio,
    },
    {
        PCI_VDEVICE_SUB(SBSMODULARIO, PCI_DEVICE_ID_OCTPRO,
                        PCI_SUBVENDOR_ID_SBSMODULARIO, PCI_SUBDEVICE_ID_POCTAL422),
        .driver_data = pbn_sbsxrsio,
    },

    /*
     * Digitan DS560-558, from jimd@esoft.com
     */
    {
        PCI_VDEVICE(ATT, PCI_DEVICE_ID_ATT_VENUS_MODEM),
        .driver_data = pbn_b1_1_115200,
    },

    /*
     * Titan Electronic cards
     *  The 400L and 800L have a custom setup quirk.
     */
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_100),
        .driver_data = pbn_b0_1_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_200),
        .driver_data = pbn_b0_2_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_400),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800B),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_100L),
        .driver_data = pbn_b1_1_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_200L),
        .driver_data = pbn_b1_bt_2_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_400L),
        .driver_data = pbn_b0_bt_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800L),
        .driver_data = pbn_b0_bt_8_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_200I),
        .driver_data = pbn_b4_bt_2_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_400I),
        .driver_data = pbn_b4_bt_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800I),
        .driver_data = pbn_b4_bt_8_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_400EH),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800EH),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800EHB),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_100E),
        .driver_data = pbn_titan_1_4000000,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_200E),
        .driver_data = pbn_titan_2_4000000,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_400E),
        .driver_data = pbn_titan_4_4000000,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800E),
        .driver_data = pbn_titan_8_4000000,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_200EI),
        .driver_data = pbn_titan_2_4000000,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_200EISI),
        .driver_data = pbn_titan_2_4000000,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_200V3),
        .driver_data = pbn_b0_bt_2_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_400V3),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_410V3),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800V3),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(TITAN, PCI_DEVICE_ID_TITAN_800V3B),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_1S_10x_550),
        .driver_data = pbn_b2_1_460800,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_1S_10x_650),
        .driver_data = pbn_b2_1_460800,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_1S_10x_850),
        .driver_data = pbn_b2_1_460800,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_2S_10x_550),
        .driver_data = pbn_b2_bt_2_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_2S_10x_650),
        .driver_data = pbn_b2_bt_2_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_2S_10x_850),
        .driver_data = pbn_b2_bt_2_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_4S_10x_550),
        .driver_data = pbn_b2_bt_4_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_4S_10x_650),
        .driver_data = pbn_b2_bt_4_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_4S_10x_850),
        .driver_data = pbn_b2_bt_4_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_1S_20x_550),
        .driver_data = pbn_b0_1_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_1S_20x_650),
        .driver_data = pbn_b0_1_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_1S_20x_850),
        .driver_data = pbn_b0_1_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_2S_20x_550),
        .driver_data = pbn_b0_bt_2_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_2S_20x_650),
        .driver_data = pbn_b0_bt_2_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_2S_20x_850),
        .driver_data = pbn_b0_bt_2_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_4S_20x_550),
        .driver_data = pbn_b0_bt_4_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_4S_20x_650),
        .driver_data = pbn_b0_bt_4_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_4S_20x_850),
        .driver_data = pbn_b0_bt_4_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_8S_20x_550),
        .driver_data = pbn_b0_bt_8_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_8S_20x_650),
        .driver_data = pbn_b0_bt_8_921600,
    },
    {
        PCI_VDEVICE(SIIG, PCI_DEVICE_ID_SIIG_8S_20x_850),
        .driver_data = pbn_b0_bt_8_921600,
    },

    /*
     * Computone devices submitted by Doug McNash dmcnash@computone.com
     */
    {
        PCI_VDEVICE_SUB(COMPUTONE, PCI_DEVICE_ID_COMPUTONE_PG,
                        PCI_SUBVENDOR_ID_COMPUTONE, PCI_SUBDEVICE_ID_COMPUTONE_PG4),
        .driver_data = pbn_computone_4,
    },
    {
        PCI_VDEVICE_SUB(COMPUTONE, PCI_DEVICE_ID_COMPUTONE_PG,
                        PCI_SUBVENDOR_ID_COMPUTONE, PCI_SUBDEVICE_ID_COMPUTONE_PG8),
        .driver_data = pbn_computone_8,
    },
    {
        PCI_VDEVICE_SUB(COMPUTONE, PCI_DEVICE_ID_COMPUTONE_PG,
                        PCI_SUBVENDOR_ID_COMPUTONE, PCI_SUBDEVICE_ID_COMPUTONE_PG6),
        .driver_data = pbn_computone_6,
    },

    {
        PCI_VDEVICE(OXSEMI, PCI_DEVICE_ID_OXSEMI_16PCI95N),
        .driver_data = pbn_oxsemi,
    },
    {
        PCI_VDEVICE_SUB(TIMEDIA, PCI_DEVICE_ID_TIMEDIA_1889,
                        PCI_VENDOR_ID_TIMEDIA, PCI_ANY_ID),
        .driver_data = pbn_b0_bt_1_921600,
    },

    /*
     * Sunix PCI serial boards
     */
    {
        PCI_VDEVICE_SUB(SUNIX, PCI_DEVICE_ID_SUNIX_1999,
                        PCI_VENDOR_ID_SUNIX, 0x0001),
        .driver_data = pbn_sunix_pci_1s,
    },
    {
        PCI_VDEVICE_SUB(SUNIX, PCI_DEVICE_ID_SUNIX_1999,
                        PCI_VENDOR_ID_SUNIX, 0x0002),
        .driver_data = pbn_sunix_pci_2s,
    },
    {
        PCI_VDEVICE_SUB(SUNIX, PCI_DEVICE_ID_SUNIX_1999,
                        PCI_VENDOR_ID_SUNIX, 0x0004),
        .driver_data = pbn_sunix_pci_4s,
    },
    {
        PCI_VDEVICE_SUB(SUNIX, PCI_DEVICE_ID_SUNIX_1999,
                        PCI_VENDOR_ID_SUNIX, 0x0084),
        .driver_data = pbn_sunix_pci_4s,
    },
    {
        PCI_VDEVICE_SUB(SUNIX, PCI_DEVICE_ID_SUNIX_1999,
                        PCI_VENDOR_ID_SUNIX, 0x0008),
        .driver_data = pbn_sunix_pci_8s,
    },
    {
        PCI_VDEVICE_SUB(SUNIX, PCI_DEVICE_ID_SUNIX_1999,
                        PCI_VENDOR_ID_SUNIX, 0x0088),
        .driver_data = pbn_sunix_pci_8s,
    },
    {
        PCI_VDEVICE_SUB(SUNIX, PCI_DEVICE_ID_SUNIX_1999,
                        PCI_VENDOR_ID_SUNIX, 0x0010),
        .driver_data = pbn_sunix_pci_16s,
    },

    /*
     * AFAVLAB serial card, from Harald Welte <laforge@gnumonks.org>
     */
    {
        PCI_VDEVICE(AFAVLAB, PCI_DEVICE_ID_AFAVLAB_P028),
        .driver_data = pbn_b0_bt_8_115200,
    },
    {
        PCI_VDEVICE(AFAVLAB, PCI_DEVICE_ID_AFAVLAB_P030),
        .driver_data = pbn_b0_bt_8_115200,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_DSERIAL),
        .driver_data = pbn_b0_bt_2_115200,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_QUATRO_A),
        .driver_data = pbn_b0_bt_2_115200,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_QUATRO_B),
        .driver_data = pbn_b0_bt_2_115200,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_OCTO_A),
        .driver_data = pbn_b0_bt_4_460800,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_OCTO_B),
        .driver_data = pbn_b0_bt_4_460800,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_PORT_PLUS),
        .driver_data = pbn_b0_bt_2_460800,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_QUAD_A),
        .driver_data = pbn_b0_bt_2_460800,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_QUAD_B),
        .driver_data = pbn_b0_bt_2_460800,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_SSERIAL),
        .driver_data = pbn_b0_bt_1_115200,
    },
    {
        PCI_VDEVICE(LAVA, PCI_DEVICE_ID_LAVA_PORT_650),
        .driver_data = pbn_b0_bt_1_460800,
    },

    /*
     * Korenix Jetcard F0/F1 cards (JC1204, JC1208, JC1404, JC1408).
     * Cards are identified by their subsystem vendor IDs, which
     * (in hex) match the model number.
     *
     * Note that JC140x are RS422/485 cards which require ox950
     * ACR = 0x10, and as such are not currently fully supported.
     */
    {
        PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF0,
                        0x1204, 0x0004),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF0,
                        0x1208, 0x0004),
        .driver_data = pbn_b0_4_921600,
    },
    /*	{
            PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF0,
                    0x1402, 0x0002),
            .driver_data = pbn_b0_2_921600,
        }, */
    /*	{
            PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF0,
                    0x1404, 0x0004),
            .driver_data = pbn_b0_4_921600,
        }, */
    {
        PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF1,
                        0x1208, 0x0004),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF2,
                        0x1204, 0x0004),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF2,
                        0x1208, 0x0004),
        .driver_data = pbn_b0_4_921600,
    },
    {
        PCI_VDEVICE_SUB(KORENIX, PCI_DEVICE_ID_KORENIX_JETCARDF3,
                        0x1208, 0x0004),
        .driver_data = pbn_b0_4_921600,
    },
    /*
     * Dell Remote Access Card 4 - Tim_T_Murphy@Dell.com
     */
    {
        PCI_VDEVICE(DELL, PCI_DEVICE_ID_DELL_RAC4),
        .driver_data = pbn_b1_1_1382400,
    },

    /*
     * Dell Remote Access Card III - Tim_T_Murphy@Dell.com
     */
    {
        PCI_VDEVICE(DELL, PCI_DEVICE_ID_DELL_RACIII),
        .driver_data = pbn_b1_1_1382400,
    },

    /*
     * RAStel 2 port modem, gerg@moreton.com.au
     */
    {
        PCI_VDEVICE(MORETON, PCI_DEVICE_ID_RASTEL_2PORT),
        .driver_data = pbn_b2_bt_2_115200,
    },

    /*
     * EKF addition for i960 Boards form EKF with serial port
     */
    {
        PCI_VDEVICE_SUB(INTEL, PCI_DEVICE_ID_INTEL_80960_RP,
                        0xE4BF, PCI_ANY_ID),
        .driver_data = pbn_intel_i960,
    },

    /*
     * Xircom Cardbus/Ethernet combos
     */
    {
        PCI_VDEVICE(XIRCOM, PCI_DEVICE_ID_XIRCOM_X3201_MDM),
        .driver_data = pbn_b0_1_115200,
    },
    /*
     * Xircom RBM56G cardbus modem - Dirk Arnold (temp entry)
     */
    {
        PCI_VDEVICE(XIRCOM, PCI_DEVICE_ID_XIRCOM_RBM56G),
        .driver_data = pbn_b0_1_115200,
    },

    /*
     * Untested PCI modems, sent in from various folks...
     */

    {
        /* Elsa Model 56K PCI Modem, from Andreas Rath <arh@01019freenet.de> */
        PCI_VDEVICE_SUB(ROCKWELL, 0x1004, 0x1048, 0x1500),
        .driver_data = pbn_b1_1_115200,
    },
    {
        PCI_VDEVICE_SUB(SGI, PCI_DEVICE_ID_SGI_IOC3, 0xFF00, 0),
        .driver_data = pbn_sgi_ioc3,
    },

    /*
     * HP Diva card
     */
    {
        PCI_VDEVICE_SUB(HP, PCI_DEVICE_ID_HP_DIVA,
                        PCI_VENDOR_ID_HP, PCI_DEVICE_ID_HP_DIVA_RMP3),
        .driver_data = pbn_b1_1_115200,
    },
    {
        PCI_VDEVICE(HP, PCI_DEVICE_ID_HP_DIVA),
        .driver_data = pbn_b0_5_115200,
    },
    {
        PCI_VDEVICE(HP, PCI_DEVICE_ID_HP_DIVA_AUX),
        .driver_data = pbn_b2_1_115200,
    },
    {
        /* HPE PCI serial device */
        PCI_VDEVICE(HP_3PAR, PCI_DEVICE_ID_HPE_PCI_SERIAL),
        .driver_data = pbn_b1_1_115200,
    },
    {
        PCI_VDEVICE(DCI, PCI_DEVICE_ID_DCI_PCCOM2),
        .driver_data = pbn_b3_2_115200,
    },
    {
        PCI_VDEVICE(DCI, PCI_DEVICE_ID_DCI_PCCOM4),
        .driver_data = pbn_b3_4_115200,
    },
    {
        PCI_VDEVICE(DCI, PCI_DEVICE_ID_DCI_PCCOM8),
        .driver_data = pbn_b3_8_115200,
    },
    {
        /* Topic TP560 Data/Fax/Voice 56k modem (reported by Evan Clarke) */
        PCI_VDEVICE(TOPIC, PCI_DEVICE_ID_TOPIC_TP560),
        .driver_data = pbn_b0_1_115200,
    },
    {
        /* ITE */
        PCI_VDEVICE(ITE, PCI_DEVICE_ID_ITE_8872),
        .driver_data = pbn_b1_bt_1_115200,
    },
    {
        /* IntaShield IS-100 */
        PCI_VDEVICE(INTASHIELD, 0x0D60),
        .driver_data = pbn_b2_1_115200,
    },
    {
        /* IntaShield IS-200; 135a.0d80 */
        PCI_VDEVICE(INTASHIELD, PCI_DEVICE_ID_INTASHIELD_IS200),
        .driver_data = pbn_b2_2_115200,
    },
    {
        /* IntaShield IS-400; 135a.0dc0 */
        PCI_VDEVICE(INTASHIELD, PCI_DEVICE_ID_INTASHIELD_IS400),
        .driver_data = pbn_b2_4_115200,
    },
    {
        /* IntaShield IX-100 */
        PCI_VDEVICE(INTASHIELD, 0x4027),
        .driver_data = pbn_oxsemi_1_15625000,
    },
    {
        /* IntaShield IX-200 */
        PCI_VDEVICE(INTASHIELD, 0x4028),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    {
        /* IntaShield IX-400 */
        PCI_VDEVICE(INTASHIELD, 0x4029),
        .driver_data = pbn_oxsemi_4_15625000,
    },
    /* Brainboxes Devices */
    /*
     * Brainboxes UC-101
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0BA1),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0BA2),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0BA3),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UC-235/246
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0AA1),
        .driver_data = pbn_b2_1_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0AA2),
        .driver_data = pbn_b2_1_115200,
    },
    /*
     * Brainboxes UC-253/UC-734
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0CA1),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UC-260/271/701/756
     */
    {
        PCI_DEVICE(PCI_VENDOR_ID_INTASHIELD, 0x0D21),
        .class = PCI_CLASS_COMMUNICATION_MULTISERIAL << 8,
        .class_mask = 0xFFFF00,
        .driver_data = pbn_b2_4_115200,
    },
    {
        PCI_DEVICE(PCI_VENDOR_ID_INTASHIELD, 0x0E34),
        .class = PCI_CLASS_COMMUNICATION_MULTISERIAL << 8,
        .class_mask = 0xFFFF00,
        .driver_data = pbn_b2_4_115200,
    },
    /*
     * Brainboxes UC-268
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0841),
        .driver_data = pbn_b2_4_115200,
    },
    /*
     * Brainboxes UC-275/279
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0881),
        .driver_data = pbn_b2_8_115200,
    },
    /*
     * Brainboxes UC-302
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x08E1),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x08E2),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x08E3),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UC-310
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x08C1),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UC-313
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x08A1),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x08A2),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x08A3),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UC-320/324
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0A61),
        .driver_data = pbn_b2_1_115200,
    },
    /*
     * Brainboxes UC-346
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0B01),
        .driver_data = pbn_b2_4_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0B02),
        .driver_data = pbn_b2_4_115200,
    },
    /*
     * Brainboxes UC-357
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0A81),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0A82),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0A83),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UC-368
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0C41),
        .driver_data = pbn_b2_4_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0C42),
        .driver_data = pbn_b2_4_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0C43),
        .driver_data = pbn_b2_4_115200,
    },
    /*
     * Brainboxes UC-420
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0921),
        .driver_data = pbn_b2_4_115200,
    },
    /*
     * Brainboxes UC-607
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x09A1),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x09A2),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x09A3),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UC-836
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0D41),
        .driver_data = pbn_b2_4_115200,
    },
    /*
     * Brainboxes UP-189
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0AC1),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0AC2),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0AC3),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UP-200
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0B21),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0B22),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0B23),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UP-869
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0C01),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0C02),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0C03),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes UP-880
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0C21),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0C22),
        .driver_data = pbn_b2_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x0C23),
        .driver_data = pbn_b2_2_115200,
    },
    /*
     * Brainboxes PX-101
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4005),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x4019),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    /*
     * Brainboxes PX-235/246
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4004),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x4016),
        .driver_data = pbn_oxsemi_1_15625000,
    },
    /*
     * Brainboxes PX-203/PX-257
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4006),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x4015),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    /*
     * Brainboxes PX-260/PX-701
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x400A),
        .driver_data = pbn_oxsemi_4_15625000,
    },
    /*
     * Brainboxes PX-275/279
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x0E41),
        .driver_data = pbn_b2_8_115200,
    },
    /*
     * Brainboxes PX-310
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x400E),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    /*
     * Brainboxes PX-313
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x400C),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    /*
     * Brainboxes PX-320/324/PX-376/PX-387
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x400B),
        .driver_data = pbn_oxsemi_1_15625000,
    },
    /*
     * Brainboxes PX-335/346
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x400F),
        .driver_data = pbn_oxsemi_4_15625000,
    },
    /*
     * Brainboxes PX-368
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4010),
        .driver_data = pbn_oxsemi_4_15625000,
    },
    /*
     * Brainboxes PX-420
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4000),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x4011),
        .driver_data = pbn_oxsemi_4_15625000,
    },
    /*
     * Brainboxes PX-475
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x401D),
        .driver_data = pbn_oxsemi_1_15625000,
    },
    /*
     * Brainboxes PX-803/PX-857
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4009),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x4018),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x401E),
        .driver_data = pbn_oxsemi_2_15625000,
    },
    /*
     * Brainboxes PX-820
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4002),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x4013),
        .driver_data = pbn_oxsemi_4_15625000,
    },
    /*
     * Brainboxes PX-835/PX-846
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4008),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE(INTASHIELD, 0x4017),
        .driver_data = pbn_oxsemi_1_15625000,
    },
    /*
     * Brainboxes XC-235
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4026),
        .driver_data = pbn_oxsemi_1_15625000,
    },
    /*
     * Brainboxes XC-475
     */
    {
        PCI_VDEVICE(INTASHIELD, 0x4021),
        .driver_data = pbn_oxsemi_1_15625000,
    },

    /*
     * Perle PCI-RAS cards
     */
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9030,
                        PCI_SUBVENDOR_ID_PERLE, PCI_SUBDEVICE_ID_PCI_RAS4),
        .driver_data = pbn_b2_4_921600,
    },
    {
        PCI_VDEVICE_SUB(PLX, PCI_DEVICE_ID_PLX_9030,
                        PCI_SUBVENDOR_ID_PERLE, PCI_SUBDEVICE_ID_PCI_RAS8),
        .driver_data = pbn_b2_8_921600,
    },

    /*
     * Mainpine series cards: Fairly standard layout but fools
     * parts of the autodetect in some cases and uses otherwise
     * unmatched communications subclasses in the PCI Express case
     */

    {
        /* RockForceDUO */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0200),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* RockForceQUATRO */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0300),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* RockForceDUO+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0400),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* RockForceQUATRO+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0500),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* RockForce+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0600),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* RockForce+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0700),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* RockForceOCTO+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0800),
        .driver_data = pbn_b0_8_115200,
    },
    {
        /* RockForceDUO+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0C00),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* RockForceQUARTRO+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x0D00),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* RockForceOCTO+ */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x1D00),
        .driver_data = pbn_b0_8_115200,
    },
    {
        /* RockForceD1 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        /* RockForceF1 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2100),
        .driver_data = pbn_b0_1_115200,
    },
    {
        /* RockForceD2 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2200),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* RockForceF2 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2300),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* RockForceD4 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2400),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* RockForceF4 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2500),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* RockForceD8 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2600),
        .driver_data = pbn_b0_8_115200,
    },
    {
        /* RockForceF8 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x2700),
        .driver_data = pbn_b0_8_115200,
    },
    {
        /* IQ Express D1 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        /* IQ Express F1 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3100),
        .driver_data = pbn_b0_1_115200,
    },
    {
        /* IQ Express D2 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3200),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* IQ Express F2 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3300),
        .driver_data = pbn_b0_2_115200,
    },
    {
        /* IQ Express D4 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3400),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* IQ Express F4 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3500),
        .driver_data = pbn_b0_4_115200,
    },
    {
        /* IQ Express D8 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3C00),
        .driver_data = pbn_b0_8_115200,
    },
    {
        /* IQ Express F8 */
        PCI_VDEVICE_SUB(MAINPINE, PCI_DEVICE_ID_MAINPINE_PBRIDGE,
                        PCI_VENDOR_ID_MAINPINE, 0x3D00),
        .driver_data = pbn_b0_8_115200,
    },

    {
        /* PA Semi PA6T-1682M on-chip UART */
        PCI_VDEVICE(PASEMI, 0xA004),
        .driver_data = pbn_pasemi_1682M,
    },

    /*
     * National Instruments
     */
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI23216),
        .driver_data = pbn_b1_16_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI2328),
        .driver_data = pbn_b1_8_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI2324),
        .driver_data = pbn_b1_bt_4_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI2322),
        .driver_data = pbn_b1_bt_2_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI2324I),
        .driver_data = pbn_b1_bt_4_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI2322I),
        .driver_data = pbn_b1_bt_2_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8420_23216),
        .driver_data = pbn_b1_16_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8420_2328),
        .driver_data = pbn_b1_8_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8420_2324),
        .driver_data = pbn_b1_bt_4_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8420_2322),
        .driver_data = pbn_b1_bt_2_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8422_2324),
        .driver_data = pbn_b1_bt_4_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8422_2322),
        .driver_data = pbn_b1_bt_2_115200,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8430_2322),
        .driver_data = pbn_ni8430_2,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI8430_2322),
        .driver_data = pbn_ni8430_2,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8430_2324),
        .driver_data = pbn_ni8430_4,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI8430_2324),
        .driver_data = pbn_ni8430_4,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8430_2328),
        .driver_data = pbn_ni8430_8,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI8430_2328),
        .driver_data = pbn_ni8430_8,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8430_23216),
        .driver_data = pbn_ni8430_16,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI8430_23216),
        .driver_data = pbn_ni8430_16,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8432_2322),
        .driver_data = pbn_ni8430_2,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI8432_2322),
        .driver_data = pbn_ni8430_2,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PXI8432_2324),
        .driver_data = pbn_ni8430_4,
    },
    {
        PCI_VDEVICE(NI, PCI_DEVICE_ID_NI_PCI8432_2324),
        .driver_data = pbn_ni8430_4,
    },

    /*
     * ADDI-DATA GmbH communication cards <info@addi-data.com>
     */
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7500),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7420),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7300),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE(AMCC, PCI_DEVICE_ID_AMCC_ADDIDATA_APCI7800),
        .driver_data = pbn_b1_8_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7500_2),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7420_2),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7300_2),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7500_3),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7420_3),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7300_3),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCI7800_3),
        .driver_data = pbn_b0_8_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCIe7500),
        .driver_data = pbn_ADDIDATA_PCIe_4_3906250,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCIe7420),
        .driver_data = pbn_ADDIDATA_PCIe_2_3906250,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCIe7300),
        .driver_data = pbn_ADDIDATA_PCIe_1_3906250,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_APCIe7800),
        .driver_data = pbn_ADDIDATA_PCIe_8_3906250,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_CPCI7500),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_CPCI7500_NG),
        .driver_data = pbn_b0_4_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_CPCI7420_NG),
        .driver_data = pbn_b0_2_115200,
    },
    {
        PCI_VDEVICE(ADDIDATA, PCI_DEVICE_ID_ADDIDATA_CPCI7300_NG),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9835,
                        PCI_VENDOR_ID_IBM, 0x0299),
        .driver_data = pbn_b0_bt_2_115200,
    },

    /*
     * other NetMos 9835 devices are most likely handled by the
     * parport_serial driver, check drivers/parport/parport_serial.c
     * before adding them here.
     */

    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9901,
                        0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        /* the 9901 is a rebranded 9912 */
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9912,
                        0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9922,
                        0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9904,
                        0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9900,
                        0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9900,
                        0xA000, 0x3002),
        .driver_data = pbn_NETMOS9900_2s_115200,
    },
    {
        PCI_DEVICE_SUB(PCIE_VENDOR_ID_ASIX, PCIE_DEVICE_ID_AX99100,
                       0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },

    /*
     * Best Connectivity and Rosewill PCI Multi I/O cards
     */

    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9865,
                        0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9865,
                        0xA000, 0x3002),
        .driver_data = pbn_b0_bt_2_115200,
    },
    {
        PCI_VDEVICE_SUB(NETMOS, PCI_DEVICE_ID_NETMOS_9865,
                        0xA000, 0x3004),
        .driver_data = pbn_b0_bt_4_115200,
    },

    /*
     * ASIX AX99100 PCIe to Multi I/O Controller
     */
    {
        PCI_VDEVICE_SUB(ASIX, PCI_DEVICE_ID_ASIX_AX99100,
                        0xA000, 0x1000),
        .driver_data = pbn_b0_1_115200,
    },

    /* Intel CE4100 */
    {
        PCI_VDEVICE(INTEL, PCI_DEVICE_ID_INTEL_CE4100_UART),
        .driver_data = pbn_ce4100_1_115200,
    },

    /*
     * Cronyx Omega PCI
     */
    {
        PCI_VDEVICE(PLX, PCI_DEVICE_ID_PLX_CRONYX_OMEGA),
        .driver_data = pbn_omegapci,
    },

    /*
     * Broadcom TruManage
     */
    {
        PCI_VDEVICE(BROADCOM, PCI_DEVICE_ID_BROADCOM_TRUMANAGE),
        .driver_data = pbn_brcm_trumanage,
    },

    /*
     * AgeStar as-prs2-009
     */
    {
        PCI_VDEVICE(AGESTAR, PCI_DEVICE_ID_AGESTAR_9375),
        .driver_data = pbn_b0_bt_2_115200,
    },

    /*
     * WCH CH353 series devices: The 2S1P is handled by parport_serial
     * so not listed here.
     */
    {
        PCI_VDEVICE(WCHCN, PCI_DEVICE_ID_WCHCN_CH353_4S),
        .driver_data = pbn_b0_bt_4_115200,
    },
    {
        PCI_VDEVICE(WCHCN, PCI_DEVICE_ID_WCHCN_CH353_2S1PF),
        .driver_data = pbn_b0_bt_2_115200,
    },
    {
        PCI_VDEVICE(WCHCN, PCI_DEVICE_ID_WCHCN_CH355_4S),
        .driver_data = pbn_b0_bt_4_115200,
    },
    {
        PCI_VDEVICE(WCHIC, PCI_DEVICE_ID_WCHIC_CH382_2S),
        .driver_data = pbn_wch382_2,
    },
    {
        PCI_VDEVICE(WCHIC, PCI_DEVICE_ID_WCHIC_CH384_4S),
        .driver_data = pbn_wch384_4,
    },
    {
        PCI_VDEVICE(WCHIC, PCI_DEVICE_ID_WCHIC_CH384_8S),
        .driver_data = pbn_wch384_8,
    },

    /*
     * Realtek RealManage
     */
    {
        PCI_VDEVICE(REALTEK, 0x816A),
        .driver_data = pbn_b0_1_115200,
    },
    {
        PCI_VDEVICE(REALTEK, 0x816B),
        .driver_data = pbn_b0_1_115200,
    },

    /* Systembase Multi I/O cards */
    {
        PCI_VDEVICE(SYSTEMBASE, 0x0008),
        .driver_data = pbn_b0_8_921600,
    },

    /* Fintek PCI serial cards */
    {
        PCI_DEVICE(0x1C29, 0x1104),
        .driver_data = pbn_fintek_4,
    },
    {
        PCI_DEVICE(0x1C29, 0x1108),
        .driver_data = pbn_fintek_8,
    },
    {
        PCI_DEVICE(0x1C29, 0x1112),
        .driver_data = pbn_fintek_12,
    },
    {
        PCI_DEVICE(0x1C29, 0x1204),
        .driver_data = pbn_fintek_F81504A,
    },
    {
        PCI_DEVICE(0x1C29, 0x1208),
        .driver_data = pbn_fintek_F81508A,
    },
    {
        PCI_DEVICE(0x1C29, 0x1212),
        .driver_data = pbn_fintek_F81512A,
    },

    /* MKS Tenta SCOM-080x serial cards */
    {
        PCI_DEVICE(0x1601, 0x0800),
        .driver_data = pbn_b0_4_1250000,
    },
    {
        PCI_DEVICE(0x1601, 0xA801),
        .driver_data = pbn_b0_4_1250000,
    },

    /* Amazon PCI serial device */
    {
        PCI_DEVICE(0x1D0F, 0x8250),
        .driver_data = pbn_b0_1_115200,
    },

    /*
     * These entries match devices with class COMMUNICATION_SERIAL,
     * COMMUNICATION_MODEM or COMMUNICATION_MULTISERIAL
     */
    {
        PCI_DEVICE_CLASS(PCI_CLASS_COMMUNICATION_SERIAL << 8, 0xFFFF00),
        .driver_data = pbn_default,
    },
    {
        PCI_DEVICE_CLASS(PCI_CLASS_COMMUNICATION_MODEM << 8, 0xFFFF00),
        .driver_data = pbn_default,
    },
    {
        PCI_DEVICE_CLASS(PCI_CLASS_COMMUNICATION_MULTISERIAL << 8, 0xFFFF00),
        .driver_data = pbn_default,
    },
    {}};

static pci_driver_t serial_pci_driver = {
    .name = "serial",
    .probe = pciserial_init_one,
    .remove = pciserial_remove_one,
    .id_table = serial_pci_tbl,
    // .err_handler	= &serial8250_err_handler,
};

MODULE_PCI_DRIVER(serial_pci_driver);

/*
 * File: pci_regs.h
 * File Created: 27 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <binary.h>

#define PCI_CFG_SPACE_SIZE     256
#define PCI_CFG_SPACE_EXP_SIZE 4096

#define PCI_STD_HEADER_SIZEOF  64
#define PCI_STD_NUM_BARS       6    /* Number of standard BARs */

#define PCI_HEADER_VENDOR_ID   0x00 /* 16 bits */
#define PCI_HEADER_DEVICE_ID   0x02 /* 16 bits */

#define PCI_HEADER_COMMAND     0x04 /* 16 bits */

typedef enum pci_command_bits
{
    IO_SPACE_BIT = 0,
    MEMORY_SPACE_BIT = 1,
    BUS_MASTER_BIT = 2,
    SPECIAL_CYCLES_BIT = 3,
    INVALIDATE_BIT = 4,
    VGA_PALETTE_BIT = 5,
    PARITY_BIT = 6,
    WAIT_BIT = 7,
    SERR_BIT = 8,
    FAST_BACK_BIT = 9,
    INTERRUPT_DISABLE_BIT = 10,
} pci_command_bits_t;

#define PCI_COMMAND_IO           BIT(IO_SPACE_BIT)          /* Enable response in I/O space */
#define PCI_COMMAND_MEMORY       BIT(MEMORY_SPACE_BIT)      /* Enable response in Memory space */
#define PCI_COMMAND_MASTER       BIT(BUS_MASTER_BIT)        /* Enable bus mastering */
#define PCI_COMMAND_SPECIAL      BIT(SPECIAL_CYCLES_BIT)    /* Enable response to special cycles */
#define PCI_COMMAND_INVALIDATE   BIT(INVALIDATE_BIT)        /* Use memory write and invalidate */
#define PCI_COMMAND_VGA_PALETTE  BIT(VGA_PALETTE_BIT)       /* Enable palette snooping */
#define PCI_COMMAND_PARITY       BIT(PARITY_BIT)            /* Enable parity checking */
#define PCI_COMMAND_WAIT         BIT(WAIT_BIT)              /* Enable address/data stepping */
#define PCI_COMMAND_SERR         BIT(SERR_BIT)              /* Enable SERR */
#define PCI_COMMAND_FAST_BACK    BIT(FAST_BACK_BIT)         /* Enable back-to-back writes */
#define PCI_COMMAND_INTX_DISABLE BIT(INTERRUPT_DISABLE_BIT) /* INTx Emulation Disable */

#define PCI_HEADER_STATUS        0x06                       /* 16 bits */

typedef enum pci_status_bits
{
    IMM_READY = 0,
    INTERRUPT = 3,
    CAP_LIST = 4,
    _66MHZ = 5,
    UDF = 6,
    FAST_BACK = 7,
    PARITY = 8,
    DEVSEL_START = 9,
    DEVSEL_END = 10,
    SIG_TARGET_ABORT = 11,
    REC_TARGET_ABORT = 12,
    REC_MASTER_ABORT = 13,
    SIG_SYSTEM_ERROR = 14,
    DETECTED_PARITY = 15,
} pci_status_bits_t;

#define PCI_STATUS_IMM_READY              BIT(IMM_READY)                      /* Immediate Readiness */
#define PCI_STATUS_INTERRUPT              BIT(INTERRUPT)                      /* Interrupt status */
#define PCI_STATUS_CAP_LIST               BIT(CAP_LIST)                       /* Support Capability List */
#define PCI_STATUS_66MHZ                  BIT(_66MHZ)                         /* Support 66 MHz PCI 2.1 bus */
#define PCI_STATUS_UDF                    BIT(UDF)                            /* Support User Definable Features [obsolete] */
#define PCI_STATUS_FAST_BACK              BIT(FAST_BACK)                      /* Accept fast-back to back */
#define PCI_STATUS_PARITY                 BIT(PARITY)                         /* Detected parity error */
#define PCI_STATUS_DEVSEL_MASK            BIT_RANGE(DEVSEL_END, DEVSEL_START) /* DEVSEL timing */
#define PCI_STATUS_DEVSEL_FAST            0x000
#define PCI_STATUS_DEVSEL_MEDIUM          0x200
#define PCI_STATUS_DEVSEL_SLOW            0x400
#define PCI_STATUS_SIG_TARGET_ABORT       BIT(SIG_TARGET_ABORT) /* Set on target abort */
#define PCI_STATUS_REC_TARGET_ABORT       BIT(REC_TARGET_ABORT) /* Master ack of " */
#define PCI_STATUS_REC_MASTER_ABORT       BIT(REC_MASTER_ABORT) /* Set on master abort */
#define PCI_STATUS_SIG_SYSTEM_ERROR       BIT(SIG_SYSTEM_ERROR) /* Set when we drive SERR */
#define PCI_STATUS_DETECTED_PARITY        BIT(DETECTED_PARITY)  /* Set on parity error */

#define PCI_HEADER_CLASS_REVISION         0x08                  /* High 24 bits are class, low 8 revision */
#define PCI_HEADER_REVISION_ID            0x08                  /* Revision ID */
#define PCI_HEADER_CLASS_PROG             0x09                  /* Reg. Level Programming Interface */
#define PCI_HEADER_CLASS_DEVICE           0x0A                  /* Device class */

#define PCI_HEADER_CACHE_LINE_SIZE        0x0C                  /* 8 bits */
#define PCI_HEADER_LATENCY_TIMER          0x0D                  /* 8 bits */

#define PCI_HEADER_HEADER_TYPE            0x0E                  /* 8 bits */
#define PCI_HEADER_TYPE_MASK              0x7F
#define PCI_HEADER_TYPE_NORMAL            0
#define PCI_HEADER_TYPE_BRIDGE            1
#define PCI_HEADER_TYPE_CARDBUS           2
#define PCI_HEADER_TYPE_MFD               0x80 /* Multi-Function Device (possible) */

#define PCI_HEADER_BIST                   0x0F /* 8 bits */
#define PCI_BIST_CODE_MASK                0x0F /* Return result */
#define PCI_BIST_START                    0x40 /* 1 to start BIST, 2 secs or less */
#define PCI_BIST_CAPABLE                  0x80 /* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define PCI_HEADER_BASE_ADDRESS_0         0x10 /* 32 bits */
#define PCI_HEADER_BASE_ADDRESS_1         0x14 /* 32 bits [htype 0,1 only] */
#define PCI_HEADER_BASE_ADDRESS_2         0x18 /* 32 bits [htype 0 only] */
#define PCI_HEADER_BASE_ADDRESS_3         0x1C /* 32 bits */
#define PCI_HEADER_BASE_ADDRESS_4         0x20 /* 32 bits */
#define PCI_HEADER_BASE_ADDRESS_5         0x24 /* 32 bits */
#define PCI_BASE_ADDRESS_SPACE            0x01 /* 0 = memory, 1 = I/O */
#define PCI_BASE_ADDRESS_SPACE_IO         0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY     0x00
#define PCI_BASE_ADDRESS_MEM_TYPE_MASK    0x06
#define PCI_BASE_ADDRESS_MEM_TYPE_32      0x00 /* 32 bit address */
#define PCI_BASE_ADDRESS_MEM_TYPE_1M      0x02 /* Below 1M [obsolete] */
#define PCI_BASE_ADDRESS_MEM_TYPE_64      0x04 /* 64 bit address */
#define PCI_BASE_ADDRESS_MEM_PREFETCH     0x08 /* prefetchable? */
#define PCI_BASE_ADDRESS_MEM_MASK         (~0x0Ful)
#define PCI_BASE_ADDRESS_IO_MASK          (~0x03ul)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#define PCI_HEADER_CARDBUS_CIS            0x28
#define PCI_HEADER_SUBSYSTEM_VENDOR_ID    0x2C
#define PCI_HEADER_SUBSYSTEM_ID           0x2E
#define PCI_HEADER_ROM_ADDRESS            0x30 /* Bits 31..11 are address, 10..1 reserved */
#define PCI_ROM_ADDRESS_ENABLE            0x01
#define PCI_ROM_ADDRESS_MASK              (~0x7FFu)

#define PCI_HEADER_CAPABILITY_LIST        0x34 /* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#define PCI_HEADER_INTERRUPT_LINE         0x3C /* 8 bits */
#define PCI_HEADER_INTERRUPT_PIN          0x3D /* 8 bits */
#define PCI_HEADER_MIN_GNT                0x3E /* 8 bits */
#define PCI_HEADER_MAX_LAT                0x3F /* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#define PCI_HEADER_PRIMARY_BUS            0x18 /* Primary bus number */
#define PCI_HEADER_SECONDARY_BUS          0x19 /* Secondary bus number */
#define PCI_HEADER_SUBORDINATE_BUS        0x1A /* Highest bus number behind the bridge */
#define PCI_HEADER_SEC_LATENCY_TIMER      0x1B /* Latency timer for secondary interface */

/* Masks for dword-sized processing of Bus Number and Sec Latency Timer fields */
#define PCI_PRIMARY_BUS_MASK              0x000000FF
#define PCI_SECONDARY_BUS_MASK            0x0000FF00
#define PCI_SUBORDINATE_BUS_MASK          0x00FF0000
#define PCI_SEC_LATENCY_TIMER_MASK        0xFF000000

#define PCI_HEADER_IO_BASE                0x1C   /* I/O range behind the bridge */
#define PCI_HEADER_IO_LIMIT               0x1D
#define PCI_IO_RANGE_TYPE_MASK            0x0Ful /* I/O bridging type */
#define PCI_IO_RANGE_TYPE_16              0x00
#define PCI_IO_RANGE_TYPE_32              0x01
#define PCI_IO_RANGE_MASK                 (~0x0Ful) /* Standard 4K I/O windows */
#define PCI_IO_1K_RANGE_MASK              (~0x03ul) /* Intel 1K I/O windows */
#define PCI_SEC_STATUS                    0x1E      /* Secondary status register, only bit 14 used */

#define PCI_HEADER_MEMORY_BASE            0x20      /* Memory range behind */
#define PCI_HEADER_MEMORY_LIMIT           0x22
#define PCI_MEMORY_RANGE_TYPE_MASK        0x0Ful
#define PCI_MEMORY_RANGE_MASK             (~0x0Ful)

#define PCI_HEADER_PREF_MEMORY_BASE       0x24 /* Prefetchable memory range behind */
#define PCI_HEADER_PREF_MEMORY_LIMIT      0x26
#define PCI_PREF_RANGE_TYPE_MASK          0x0Ful
#define PCI_PREF_RANGE_TYPE_32            0x00
#define PCI_PREF_RANGE_TYPE_64            0x01
#define PCI_PREF_RANGE_MASK               (~0x0Ful)

#define PCI_HEADER_PREF_BASE_UPPER32      0x28 /* Upper half of prefetchable memory range */
#define PCI_HEADER_PREF_LIMIT_UPPER32     0x2C
#define PCI_HEADER_IO_BASE_UPPER16        0x30 /* Upper half of I/O addresses */
#define PCI_HEADER_IO_LIMIT_UPPER16       0x32

/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#define PCI_HEADER_ROM_ADDRESS1           0x38 /* Same as PCI_ROM_ADDRESS, but for htype 1 */

/* 0x3c-0x3d are same as for htype 0 */
#define PCI_HEADER_BRIDGE_CONTROL         0x3E
#define PCI_BRIDGE_CTL_PARITY             0x01 /* Enable parity detection on secondary interface */
#define PCI_BRIDGE_CTL_SERR               0x02 /* The same for SERR forwarding */
#define PCI_BRIDGE_CTL_ISA                0x04 /* Enable ISA mode */
#define PCI_BRIDGE_CTL_VGA                0x08 /* Forward VGA addresses */
#define PCI_BRIDGE_CTL_MASTER_ABORT       0x20 /* Report master aborts */
#define PCI_BRIDGE_CTL_BUS_RESET          0x40 /* Secondary bus reset */
#define PCI_BRIDGE_CTL_FAST_BACK          0x80 /* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#define PCI_HEADER_CB_CAPABILITY_LIST     0x14
/* 0x15 reserved */
#define PCI_HEADER_CB_SEC_STATUS          0x16 /* Secondary status */
#define PCI_HEADER_CB_PRIMARY_BUS         0x18 /* PCI bus number */
#define PCI_HEADER_CB_CARD_BUS            0x19 /* CardBus bus number */
#define PCI_HEADER_CB_SUBORDINATE_BUS     0x1A /* Subordinate bus number */
#define PCI_HEADER_CB_LATENCY_TIMER       0x1B /* CardBus latency timer */
#define PCI_HEADER_CB_MEMORY_BASE_0       0x1C
#define PCI_HEADER_CB_MEMORY_LIMIT_0      0x20
#define PCI_HEADER_CB_MEMORY_BASE_1       0x24
#define PCI_HEADER_CB_MEMORY_LIMIT_1      0x28
#define PCI_HEADER_CB_IO_BASE_0           0x2C
#define PCI_HEADER_CB_IO_BASE_0_HI        0x2E
#define PCI_HEADER_CB_IO_LIMIT_0          0x30
#define PCI_HEADER_CB_IO_LIMIT_0_HI       0x32
#define PCI_HEADER_CB_IO_BASE_1           0x34
#define PCI_HEADER_CB_IO_BASE_1_HI        0x36
#define PCI_HEADER_CB_IO_LIMIT_1          0x38
#define PCI_HEADER_CB_IO_LIMIT_1_HI       0x3A
#define PCI_CB_IO_RANGE_MASK              (~0x03ul)
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_HEADER_CB_BRIDGE_CONTROL      0x3E
#define PCI_CB_BRIDGE_CTL_PARITY          0x01 /* Similar to standard bridge control register */
#define PCI_CB_BRIDGE_CTL_SERR            0x02
#define PCI_CB_BRIDGE_CTL_ISA             0x04
#define PCI_CB_BRIDGE_CTL_VGA             0x08
#define PCI_CB_BRIDGE_CTL_MASTER_ABORT    0x20
#define PCI_CB_BRIDGE_CTL_CB_RESET        0x40  /* CardBus reset */
#define PCI_CB_BRIDGE_CTL_16BIT_INT       0x80  /* Enable interrupt for 16-bit cards */
#define PCI_CB_BRIDGE_CTL_PREFETCH_MEM0   0x100 /* Prefetch enable for both memory regions */
#define PCI_CB_BRIDGE_CTL_PREFETCH_MEM1   0x200
#define PCI_CB_BRIDGE_CTL_POST_WRITES     0x400
#define PCI_HEADER_CB_SUBSYSTEM_VENDOR_ID 0x40
#define PCI_HEADER_CB_SUBSYSTEM_ID        0x42
#define PCI_HEADER_CB_LEGACY_MODE_BASE    0x44 /* 16-bit PC Card legacy mode base address (ExCa) */
                                               /* 0x48-0x7f reserved */

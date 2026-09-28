/*
 * File: resourcet.h
 * File Created: 27 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <binary.h>
#include <types.h>

typedef paddr_t resource_size_t;

/**
 * @brief Address space a resource range lives in.
 * @note Distinct spaces never overlap each other — a tree root per space.
 */
typedef enum
{
    RES_SPACE_MEM, /**< Physical memory-mapped address space (MMIO + RAM) */
    RES_SPACE_IO,  /**< I/O port space */
    RES_SPACE_IRQ, /**< IRQ/GSI number space */
} res_space_t;

/**
 * @brief What a claimed range represents. Coarser than Linux's IORESOURCE_*
 *        flags — enough to answer "can I reserve over/under this" and
 *        "what do I print in a debug dump", not full hotplug semantics.
 */
typedef enum
{
    RES_TYPE_RAM,            /**< Usable system RAM */
    RES_TYPE_RESERVED,       /**< Firmware/platform reserved, not for allocation */
    RES_TYPE_ACPI_TABLES,    /**< ACPI table blob, lives inside RAM already owned */
    RES_TYPE_PCI_BUS,        /**< A PCI bus's MMIO/IO window */
    RES_TYPE_PCI_BAR,        /**< A single BAR belonging to one BDF */
    RES_TYPE_PLATFORM_FIXED, /**< Fixed platform device: IOAPIC, HPET, etc. */
    RES_TYPE_KERNEL_IMAGE,   /**< Kernel code/rodata/data/bss segments */
} res_type_t;

/**
 * @brief Bitmask modifiers, orthogonal to res_type_t.
 */
typedef enum
{
    RES_FLAG_NONE = 0,
    RES_FLAG_READONLY = BIT(0),
    RES_FLAG_PREFETCH = BIT(1), /**< PCI prefetchable BAR */
    RES_FLAG_BUSY = BIT(2),     /**< Claimed and in active use (vs reserved-but-free) */
} res_flags_t;

/**
 * @brief One node in a per-space resource tree. Ranges are inclusive
 *        [start, end]. A node's range must fully contain all its children's
 *        ranges; siblings under the same parent must not overlap.
 */
typedef struct resource
{
    resource_size_t start;
    resource_size_t end;
    const char *name; /**< Owner tag, e.g. "0000:06:00.0" or "System RAM" */
    res_type_t type;
    res_flags_t flags;

    /* PCI-specific identity, only meaningful when type == RES_TYPE_PCI_BAR
       or RES_TYPE_PCI_BUS. Kept inline rather than a union to avoid an
       extra indirection on the hot lookup path; revisit if this struct
       grows more per-type payloads. */
    uint8_t pci_bus;
    uint8_t pci_dev;
    uint8_t pci_func;
    uint8_t pci_bar_index; /**< 0-5; ignored for RES_TYPE_PCI_BUS */

    struct resource *parent;
    struct resource *sibling; /**< Next sibling under the same parent */
    struct resource *child;   /**< First child */
} resource_t;

/**
 * @brief Root resource for one address space. One per res_space_t value.
 * @pre Must be initialized via resource_space_init() before any claim call.
 */
typedef struct
{
    resource_t root;
    res_space_t space;
} resource_space_t;

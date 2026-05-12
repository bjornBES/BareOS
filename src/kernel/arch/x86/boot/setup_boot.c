/*
 * File: setup_boot.c
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "setup_boot.h"
#include "libs/malloc.h"
#include "memory/paging/paging.h"

boot_params *boot_move_params(boot_params *bootParams)
{
    log_info("BOOT", "====Moving boot params====");
    boot_params *bp = malloc(sizeof(boot_params));
    boot_params *virt_bootParams = (boot_params *)((uint64_t)bootParams + PAGE_SIZE);
    paging_map_region(kernel_page, (virt_addr)PAGE_SIZE, (phys_addr)0, sizeof(boot_params) + PAGE_SIZE, kernel_text_flags);
    log_debug("BOOT", "Mapped boot params");
    log_debug("BOOT", "Copying from %p to %p", bootParams, bp);
    memcpy(bp, virt_bootParams, sizeof(boot_params));
    log_debug("BOOT", "Freeing boot params");
    paging_free_region(kernel_page, (virt_addr)virt_bootParams, sizeof(boot_params) + PAGE_SIZE);
    return bp;
}
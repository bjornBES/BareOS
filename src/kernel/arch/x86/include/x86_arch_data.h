/*
 * File: x86_arch_data.h
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/cpuid/types.h"
#include <types.h>

typedef struct x86_arch_data
{
    
    uint16_t long_mode : 1;
    uint16_t has_msr : 1;

    struct
    {
        uint16_t max_phys;
        uint16_t pse : 1;
        uint16_t pae : 1;
        uint16_t pat : 1;
        uint16_t pse_36 : 1;
        uint16_t paging_64 : 1;
        uint16_t la47 : 1;
        uint16_t huge_pdpt : 1;
        uint16_t global : 1;
        uint16_t has_nx : 1;
        uint16_t has_user_pke : 1;
        uint16_t has_super_pke : 1;
    } paging;

    cpuid_leaves_t cpuid;

} x86_arch_data_t;

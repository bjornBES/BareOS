/*
 * File: memory.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <uapi_memory.h>

#include <defs.h>
#include <stdint.h>

/// @brief A struct that contains a 32 bit segment offset address
typedef struct
{
    /// @brief The offset field
    uint16_t offset;

    /// @brief The segment field
    uint16_t segment;
} PACKED seg_off_t;

/// @brief Will make a 32 bit linear address from a 32 bit segment and offset address
/// @param seg A segment of the address
/// @param off A offset within the segment of the address
/// @return A 32 bit linear address
void *mem_segoffset_to_linear(uint16_t seg, uint16_t off);

/// @brief Will make a 32 bit segment offset address from the given 32 bit linear address
/// @param linear A 32 bit linear address
/// @return The 32 bit segment offset values of that linear address
seg_off_t mem_linear_to_segoff(void *linear);

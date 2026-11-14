/*
 * File: dma_arch.h
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "mm/dma/dmat.h"

#include <types.h>

#include "kerrno.h"
#include <config.h>

/// @defgroup contract_arch_dma
/// @brief the contract functions that the kernel will call to use the dma.

/// @ingroup kernel_dma contract_arch_dma
/// @{

/// @brief Will map the contiguous physical frames
/// @param dma
/// @param size
/// @return KERRNO_SUCCESSES on successes or an kerrno number
status_t dma_arch_map(dma_t *dma, size_t size);

/// @brief
/// @param[in] dma
/// @param[in] src
/// @param[in] size
/// @param[out] out_size
/// @return KERRNO_SUCCESSES on successes or an kerrno number
status_t dma_arch_copy_from(dma_t *dma, void *src, size_t size, size_t *out_size);

/// @brief
/// @param[in] dma
/// @param[in] dst
/// @param[in] size
/// @param[out] out_size
/// @return KERRNO_SUCCESSES on successes or an kerrno number
status_t dma_arch_copy_to(dma_t *dma, void *dst, size_t size, size_t *out_size);

#if CONFIG_DMA == 0

status_t dma_arch_map(dma_t *dma, size_t size)
{
    trace_err("DMA", "DMA is not enabled in the configurations");
    return KERRNO_NOT_SUPPORTED;
}

status_t dma_arch_copy_from(dma_t *dma, void *src, size_t size, size_t *out_size)
{
    trace_err("DMA", "DMA is not enabled in the configurations");
    return KERRNO_NOT_SUPPORTED;
}

status_t dma_arch_copy_to(dma_t *dma, void *dst, size_t size, size_t *out_size)
{
    trace_err("DMA", "DMA is not enabled in the configurations");
    return KERRNO_NOT_SUPPORTED;
}

#endif

/// @}

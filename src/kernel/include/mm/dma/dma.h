/*
 * File: dma.h
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dmat.h"

#include "kerrno.h"
#include <types.h>
#include <config.h>

/// @defgroup kernel_dma

/// @ingroup kernel_dma
/// @{

/// @brief
/// @param[in] size
/// @param[out] out_dma
/// @return
status_t dma_allocate(size_t size, dma_t **out_dma);

/// @brief
/// @param[in] alloc
/// @param[in] src
/// @param[in] size
/// @param[out] out_size
/// @return
status_t dma_copy_from(dma_t *alloc, void *src, size_t size, size_t *out_size);

/// @brief
/// @param[in] alloc
/// @param[in] dst
/// @param[in] size
/// @param[out] out_size
/// @return
status_t dma_copy_to(dma_t *alloc, void *dst, size_t size, size_t *out_size);

#if CONFIG_DMA == 0

status_t dma_allocate(size_t size, dma_t **out_dma)
{
    trace_err("DMA", "DMA is not enabled in the configurations");
    return KERRNO_NOT_SUPPORTED;
}

status_t dma_copy_from(dma_t *alloc, void *src, size_t size, size_t *out_size)
{
    trace_err("DMA", "DMA is not enabled in the configurations");
    return KERRNO_NOT_SUPPORTED;
}

status_t dma_copy_to(dma_t *alloc, void *dst, size_t size, size_t *out_size)
{
    trace_err("DMA", "DMA is not enabled in the configurations");
    return KERRNO_NOT_SUPPORTED;
}

#endif

/// @}

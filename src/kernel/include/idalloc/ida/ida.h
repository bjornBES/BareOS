/*
 * File: ida.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "idat.h"

#include <types.h>

/// @brief 
/// @param[in] ida 
/// @return KERRNO_SUCCESS on successes or an kerrno number
/// @retval KERRNO_ALLOC_FAILED: If an allocation failed
status_t ida_init(ida_t *ida);

/// @brief 
/// @param[in] ida 
/// @return KERRNO_SUCCESS on successes or an kerrno number
status_t ida_release(ida_t *ida);

/// @brief 
/// @param[in] ida 
/// @return KERRNO_SUCCESS on successes or an kerrno number
status_t ida_release_index(ida_t *ida, int index);

/// @brief 
/// @param[in] ida 
/// @param[in] min 
/// @param[in] max 
/// @param[out] out_index 
/// @return KERRNO_SUCCESS on successes or an kerrno number
status_t ida_alloc_range(ida_t *ida, int min, int max, id_t *out_index);

/// @brief 
/// @param ida 
/// @param out_index 
/// @return KERRNO_SUCCESS on successes or an kerrno number
status_t ida_alloc(ida_t *ida, id_t *out_index);

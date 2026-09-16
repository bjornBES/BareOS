/*
 * File: ivt.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/ivt_arch.h"

#include <binary.h>
#include <types.h>

#define HANDLER_IN_USE BIT(0)
#define HANDLER_IS_ACTIVE BIT(1)

typedef status_t (*interrupt_handler)(intr_frame_t *regs);

status_t ivt_handler(interrupt_vector_t vector, intr_frame_t *frame);


/// @brief Zone all the handlers using memset
void ivt_init();

/// @brief Registers the given vector, with the given handler, so that the handler runs when an interrupt happens on that vector.
/// 
/// The function need to set the flags HANDLER_IN_USE and HANDLER_IS_ACTIVE.
/// 
/// @param[in] vector The vector that will the handler will run on.
/// @param[in] handler The handler that needs to get info about the interrupt and either resolve it, panic or give it to a kernel handler
/// 
/// @return KERRNO_SUCCESSES on successes or an kerrno number
/// @retval KERRNO_BUSY: Will be returned if the handler's HANDLER_IN_USE flag is set.
status_t ivt_set_handler(interrupt_vector_t vector, interrupt_handler handler);


/// @brief Sets the handlers HANDLER_IS_ACTIVE flag to imply that the handler can be used.
/// 
/// @param vector The vector that will be enabled.
/// 
/// @return KERRNO_SUCCESSES on successes or an kerrno number
/// @retval KERRNO_NOT_ALLOWED: Will be returned if the handler's HANDLER_IN_USE flag is zero.
status_t ivt_enable_vector(interrupt_vector_t vector);

/// @brief Unsets (zeros) the handlers HANDLER_IS_ACTIVE flag to imply that the handler cannot be used.
/// 
/// @param vector The vector that will be disabled.
/// 
/// @return KERRNO_SUCCESSES on successes or an kerrno number
/// @retval KERRNO_NOT_ALLOWED: Will be returned if the handler's HANDLER_IN_USE flag is zero.
status_t ivt_disable_vector(interrupt_vector_t vector);

/// @brief 
/// @param mask 
/// @return 
interrupt_vector_t ivt_find_free_vector(interrupt_vector_t mask);


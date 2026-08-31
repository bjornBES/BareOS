/*
 * File: ivt_arch.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

/// @file This is a test

#include "type_arch.h"

#include "int/ivt.h"

typedef int (*interrupt_handler)(intr_frame_t *regs);

/// @brief Zone all the handlers using memset
void ivt_arch_init();

/// @brief Registers the given vector, with the given handler, so that the handler runs when an interrupt happens on that vector.
/// 
/// The function need to set the flags HANDLER_IN_USE and HANDLER_IS_ACTIVE.
/// 
/// @param[in] vector The vector that will the handler will run on.
/// @param[in] handler The handler that needs to get info about the interrupt and either resolve it, panic or give it to a kernel handler
/// 
/// @retval EPERM: Will be returned if the handler's HANDLER_IN_USE flag is set.
/// @retval 0: Will be returned on successes.
int ivt_arch_set_handler(interrupt_vector vector, interrupt_handler handler);


/// @brief Sets the handlers HANDLER_IS_ACTIVE flag to imply that the handler can be used.
/// 
/// @param vector The vector that will be enabled.
/// 
/// @retval EPERM: Will be returned if the handler's HANDLER_IN_USE flag is zero.
/// @retval 0: Will be returned on successes.
int ivt_arch_enable(interrupt_vector vector);

/// @brief Unsets (zeros) the handlers HANDLER_IS_ACTIVE flag to imply that the handler cannot be used.
/// 
/// @param vector The vector that will be disabled.
/// 
/// @retval EPERM: Will be returned if the handler's HANDLER_IN_USE flag is zero.
/// @retval 0: Will be returned on successes.
int ivt_arch_disable(interrupt_vector vector);

/// @brief 
/// @param mask 
/// @return 
interrupt_vector ivt_find_free_vector(interrupt_vector mask);

/// @brief 
/// @param frame 
void ivt_dump_frame(intr_frame_t *frame);

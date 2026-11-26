/*
 * File: interrupt_flags.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <binary.h>

typedef enum interrupt_trigger
{
    INT_TRIGGER_NONE = 0,
    INT_TRIGGER_RISING = BIT(0),
    INT_TRIGGER_FALLING = BIT(1),
    INT_TRIGGER_HIGH = BIT(2),
    INT_TRIGGER_LOW = BIT(3),
} interrupt_trigger_t;

/// @brief 
typedef enum interrupt_flags
{
    /// @brief Multiple devices can share the same irq
    INT_FLAG_DEV_SHARE = BIT_ULL(8),
    /// @brief irq is per CPU
    INT_FLAG_PER_CPU = BIT_ULL(9),
} interrupt_flags_t;

#define HANDLER_IN_USE    BIT(0)
#define HANDLER_IS_ACTIVE BIT(1)
#define HANDLER_SHAREABLE BIT(2)
#define HANDLER_MSI       BIT(3)
#define HANDLER_GSI       BIT(4)
#define HANDLER_RESERVED  BIT(5)
#define HANDLER_FULL      BIT(6)

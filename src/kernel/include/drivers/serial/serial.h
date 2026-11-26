/*
 * File: serial.h
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/serial_arch.h"

#include "serial_reg.h"

#include <types.h>

#define UART_IER_ALL_INTR   (UART_IER_MSI |  \
                             UART_IER_RLSI | \
                             UART_IER_THRI | \
                             UART_IER_RDI)

#define UART_LCR_WLEN(x)    ((x) - 5)

#define UART_LSR_BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

static inline bool uart_lsr_tx_empty(uint16_t lsr)
{
    return (lsr & UART_LSR_BOTH_EMPTY) == UART_LSR_BOTH_EMPTY;
}


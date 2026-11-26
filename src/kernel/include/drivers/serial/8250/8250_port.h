/*
 * File: 8250_port.h
 * File Created: 18 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "8250_core_types.h"

#include "resource/resourcet.h"

#include <types.h>

#define UART_CAP_FIFO   BIT(8)  /* UART has FIFO */
#define UART_CAP_EFR    BIT(9)  /* UART has EFR */
#define UART_CAP_SLEEP  BIT(10) /* UART has IER sleep */
#define UART_CAP_AFE    BIT(11) /* MCR-based hw flow control */
#define UART_CAP_UUE    BIT(12) /* UART needs IER bit 6 set (Xscale) */
#define UART_CAP_RTOIE  BIT(13) /* UART needs IER bit 4 set (Xscale, Tegra) */
#define UART_CAP_HFIFO  BIT(14) /* UART has a "hidden" FIFO */
#define UART_CAP_RPM    BIT(15) /* Runtime PM is active while idle */
#define UART_CAP_IRDA   BIT(16) /* UART supports IrDA line discipline */
#define UART_CAP_MINI   BIT(17) /* Mini UART on BCM283X family lacks: STOP PARITY EPAR SPAR WLEN5 WLEN6 */
#define UART_CAP_NOTEMT BIT(18) /* UART without interrupt on TEMT available */

#define UART_BUG_QUOT   BIT(0)  /* UART has buggy quot LSB */
#define UART_BUG_TXEN   BIT(1)  /* UART has buggy TX IIR status */
#define UART_BUG_NOMSR  BIT(2)  /* UART has buggy MSR status bits (Au1x00) */
#define UART_BUG_THRE   BIT(3)  /* UART has buggy THRE reassertion */
#define UART_BUG_TXRACE BIT(5)  /* UART Tx fails to set remote DR */

#define to_uart_8250_port(n) container_of(n, uart_8250_port_t, port)

static inline status_t serial_dl_read(uart_8250_port_t *up, uint32_t *out_data)
{
    return up->dl_read(up, out_data);
}

static inline status_t serial_dl_write(uart_8250_port_t *up, uint32_t value)
{
    return up->dl_write(up, value);
}


/*
 * File: serial_port.h
 * File Created: 18 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "serial_core_types.h"
#include "serialt.h"
#include "console/consolet.h"
#include "sync/spinlock.h"

#include "resource/resourcet.h"

#include <binary.h>
#include <types.h>

#include "drivers/serial/serial_base.h"

#define UPF_NO_THRE_TEST  ((upf_t)BIT(19))
/* Port has hardware-assisted h/w flow control */
#define UPF_AUTO_CTS      ((upf_t)BIT(20))
#define UPF_AUTO_RTS      ((upf_t)BIT(21))
#define UPF_HARD_FLOW     ((upf_t)(UPF_AUTO_CTS | UPF_AUTO_RTS))
/* Port has hardware-assisted s/w flow control */
#define UPF_SOFT_FLOW     ((upf_t)BIT(22))
// bit 23
#define UPF_SHARE_IRQ     ((upf_t)BIT(24))
#define UPF_EXAR_EFR      ((upf_t)BIT(25))
#define UPF_BUG_THRE      ((upf_t)BIT(26))
/* The exact UART type is known and should not be probed.  */
#define UPF_FIXED_TYPE    ((upf_t)BIT(27))
#define UPF_BOOT_AUTOCONF ((upf_t)BIT(28))
#define UPF_FIXED_PORT    ((upf_t)BIT(29))
#define UPF_DEAD          ((upf_t)BIT(30))
#define UPF_IOREMAP       ((upf_t)BIT(31))
#define UPF_FULL_PROBE    ((upf_t)BIT(32))

#define __UPF_CHANGE_MASK	0x17fff
#define UPF_CHANGE_MASK		((__force upf_t) __UPF_CHANGE_MASK)
#define UPF_USR_MASK		((__force upf_t) (UPF_SPD_MASK|UPF_LOW_LATENCY))

// #if __UPF_CHANGE_MASK > ASYNC_FLAGS
// #error Change mask not equivalent to userspace-visible bit defines
// #endif

#define UPSTAT_CTS_ENABLE ((upstat_t)(BIT(0)))
#define UPSTAT_DCD_ENABLE ((upstat_t)(BIT(1)))
#define UPSTAT_AUTORTS    ((upstat_t)(BIT(2)))
#define UPSTAT_AUTOCTS    ((upstat_t)(BIT(3)))
#define UPSTAT_AUTOXOFF   ((upstat_t)(BIT(4)))
#define UPSTAT_SYNC_FIFO  ((upstat_t)(BIT(5)))

static inline void __uart_port_lock_irqsave(uart_port_t *up)
{
    spinlock_acquire(&up->lock);
}

static inline void __uart_port_unlock_irqrestore(uart_port_t *up)
{
    spinlock_release(&up->lock);
}

static inline void uart_port_set_cons(uart_port_t *up, console_t *con)
{
    __uart_port_lock_irqsave(up);
    up->cons = con;
    __uart_port_unlock_irqrestore(up);
}

static inline void uart_port_lock_irqsave(uart_port_t *up)
{
    spinlock_acquire(&up->lock);
    // __uart_port_nbcon_acquire(up);
}

static inline void uart_port_unlock_irqsave(uart_port_t *up)
{
    spinlock_release(&up->lock);
    // __uart_port_nbcon_acquire(up);
}

static inline status_t serial_port_in(uart_port_t *up, off_t offset, uint32_t *out_data)
{
    return up->serial_in(up, offset, out_data);
}

static inline status_t serial_port_out(uart_port_t *up, off_t offset, uint32_t value)
{
    return up->serial_out(up, offset, value);
}

static inline void uart_port_set_cons_flow_enabled(uart_port_t *uport, bool enabled)
{
    uport->cons_flow = enabled;
}

static inline bool uart_port_cons_flow_enabled(const uart_port_t *uport)
{
    return uport->cons_flow;
}


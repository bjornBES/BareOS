/*
 * File: tty_flags.h
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define ASYNCB_HUP_NOTIFY       0  /* Notify getty on hangups and closes on the callout port */
#define ASYNCB_FOURPORT         1  /* Set OUT1, OUT2 per AST Fourport settings */
#define ASYNCB_SAK              2  /* Secure Attention Key (Orange book) */
#define ASYNCB_SPLIT_TERMIOS    3  /* [x] Separate termios for dialin/callout */
#define ASYNCB_SPD_HI           4  /* Use 57600 instead of 38400 bps */
#define ASYNCB_SPD_VHI          5  /* Use 115200 instead of 38400 bps */
#define ASYNCB_SKIP_TEST        6  /* Skip UART test during autoconfiguration */
#define ASYNCB_AUTO_IRQ         7  /* Do automatic IRQ during autoconfiguration */
#define ASYNCB_SESSION_LOCKOUT  8  /* [x] Lock out cua opens based on session */
#define ASYNCB_PGRP_LOCKOUT     9  /* [x] Lock out cua opens based on pgrp */
#define ASYNCB_CALLOUT_NOHUP    10 /* [x] Don't do hangups for cua device */
#define ASYNCB_HARDPPS_CD       11 /* Call hardpps when CD goes high  */
#define ASYNCB_SPD_SHI          12 /* Use 230400 instead of 38400 bps */
#define ASYNCB_LOW_LATENCY      13 /* Request low latency behaviour */
#define ASYNCB_BUGGY_UART       14 /* This is a buggy UART, skip some safety checks.  Note: can be dangerous! */
#define ASYNCB_AUTOPROBE        15 /* [x] Port was autoprobed by PCI/PNP code */
#define ASYNCB_MAGIC_MULTIPLIER 16 /* Use special CLK or divisor */
#define ASYNCB_LAST_USER        16

#define ASYNC_HUP_NOTIFY        (1u << ASYNCB_HUP_NOTIFY)
#define ASYNC_SUSPENDED         (1u << ASYNCB_SUSPENDED)
#define ASYNC_FOURPORT          (1u << ASYNCB_FOURPORT)
#define ASYNC_SAK               (1u << ASYNCB_SAK)
#define ASYNC_SPLIT_TERMIOS     (1u << ASYNCB_SPLIT_TERMIOS)
#define ASYNC_SPD_HI            (1u << ASYNCB_SPD_HI)
#define ASYNC_SPD_VHI           (1u << ASYNCB_SPD_VHI)
#define ASYNC_SKIP_TEST         (1u << ASYNCB_SKIP_TEST)
#define ASYNC_AUTO_IRQ          (1u << ASYNCB_AUTO_IRQ)
#define ASYNC_SESSION_LOCKOUT   (1u << ASYNCB_SESSION_LOCKOUT)
#define ASYNC_PGRP_LOCKOUT      (1u << ASYNCB_PGRP_LOCKOUT)
#define ASYNC_CALLOUT_NOHUP     (1u << ASYNCB_CALLOUT_NOHUP)
#define ASYNC_HARDPPS_CD        (1u << ASYNCB_HARDPPS_CD)
#define ASYNC_SPD_SHI           (1u << ASYNCB_SPD_SHI)
#define ASYNC_LOW_LATENCY       (1u << ASYNCB_LOW_LATENCY)
#define ASYNC_BUGGY_UART        (1u << ASYNCB_BUGGY_UART)
#define ASYNC_AUTOPROBE         (1u << ASYNCB_AUTOPROBE)
#define ASYNC_MAGIC_MULTIPLIER  (1u << ASYNCB_MAGIC_MULTIPLIER)

#define ASYNC_SPD_CUST          (ASYNC_SPD_HI | ASYNC_SPD_VHI)
#define ASYNC_SPD_WARP          (ASYNC_SPD_HI | ASYNC_SPD_SHI)
#define ASYNC_SPD_MASK          (ASYNC_SPD_HI | ASYNC_SPD_VHI | ASYNC_SPD_SHI)


/*
 * File: serial.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define PORT_UNKNOWN         0
#define PORT_8250            1
#define PORT_16450           2
#define PORT_16550           3
#define PORT_16550A          4
#define PORT_CIRRUS          5
#define PORT_16650           6
#define PORT_16650V2         7
#define PORT_16750           8
#define PORT_STARTECH        9
#define PORT_16C950          10 /* Oxford Semiconductor */
#define PORT_16654           11
#define PORT_16850           12
#define PORT_RSA             13 /* RSA-DV II/S card */	
#define PORT_MAX             13

#define PORT_NS16550A        14
#define PORT_XSCALE          15
#define PORT_RM9000          16 /* PMC-Sierra RM9xxx internal UART */
#define PORT_OCTEON          17 /* Cavium OCTEON internal UART */
#define PORT_AR7             18 /* Texas Instruments AR7 internal UART */
#define PORT_U6_16550A       19 /* ST-Ericsson U6xxx internal UART */
#define PORT_TEGRA           20 /* NVIDIA Tegra internal UART */
#define PORT_XR17D15X        21 /* Exar XR17D15x UART */
#define PORT_LPC3220         22 /* NXP LPC32xx SoC "Standard" UART */
#define PORT_8250_CIR        23 /* CIR infrared port, has its own driver */
#define PORT_XR17V35X        24 /* Exar XR17V35x UARTs */
#define PORT_BRCM_TRUMANAGE  25
#define PORT_ALTR_16550_F32  26 /* Altera 16550 UART with 32 FIFOs */
#define PORT_ALTR_16550_F64  27 /* Altera 16550 UART with 64 FIFOs */
#define PORT_ALTR_16550_F128 28 /* Altera 16550 UART with 128 FIFOs */
#define PORT_RT2880          29 /* Ralink RT2880 internal UART */
#define PORT_16550A_FSL64    30 /* Freescale 16550 UART with 64 FIFOs */

#define UART_CLEAR_FIFO      0x01
#define UART_USE_FIFO        0x02
#define UART_STARTECH        0x04
#define UART_NATSEMI         0x08


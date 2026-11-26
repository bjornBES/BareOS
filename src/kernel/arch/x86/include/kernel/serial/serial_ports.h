/*
 * File: serial_ports.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

/*
 * This assumes you have a 1.8432 MHz clock for your UART.
 *
 * It'd be nice if someone built a serial card with a 24.576 MHz
 * clock, since the 16550A is capable of handling a top speed of 1.5
 * megabits/second; but this requires a faster clock.
 */
#define BASE_BAUD (1843200 / 16)

/* UART		CLK		   IRQ  FLAGS			    */
#define SERIAL_PORT_DFNS                                                     \
    {.uart = 0, BASE_BAUD, 4, 0, SERIAL_IO_TYPE_IO_PORT, .port = 0x3F8},     \
        {.uart = 0, BASE_BAUD, 3, 0, SERIAL_IO_TYPE_IO_PORT, .port = 0x2F8}, \
        {.uart = 0, BASE_BAUD, 4, 0, SERIAL_IO_TYPE_IO_PORT, .port = 0x3E8}, \
        {.uart = 0, BASE_BAUD, 3, 0, SERIAL_IO_TYPE_IO_PORT, .port = 0x2E8}

/* ttyS0 */
/* ttyS1 */
/* ttyS2 */
/* ttyS3 */

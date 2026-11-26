/*
 * File: serial_io_types.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

typedef enum serial_io_type
{
    SERIAL_IO_TYPE_UNKNOWN,
    SERIAL_IO_TYPE_IO_PORT, /* 8b I/O port access */
    SERIAL_IO_TYPE_HUB6,    /* Hub6 ISA card */
    SERIAL_IO_TYPE_MEM,     /* driver-specific */
    SERIAL_IO_TYPE_MEM16,   /* 16b little endian */
    SERIAL_IO_TYPE_MEM32,   /* 32b little endian */
    SERIAL_IO_TYPE_MEM32BE, /* 32b big endian */
    SERIAL_IO_TYPE_AU,      /* Au1x00 and RT288x type IO */
    SERIAL_IO_TYPE_TSI,     /* Tsi108/109 type IO */
    SERIAL_IO_TYPE_BUS,     /* Serial bus I/O access (ex: SPI, I2C) */
} serial_io_type_t;
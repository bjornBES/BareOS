/*
 * File: earlycon.h
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "consolet.h"

#include "drivers/serial/serial_port.h"

#include <types.h>
#include <defs.h>

typedef struct earlycon_device
{
    console_t *con;
    uart_port_t port;
    char options[32]; /* e.g., 115200n8 */
    unsigned int baud;
} earlycon_device_t;

typedef struct earlycon_id
{
    char name[15];
    char name_term; /* In case compiler didn't '\0' term name */
    char compatible[128];
    status_t (*setup)(earlycon_device_t *, const char *options);
} earlycon_id_t;

#define OF_EARLYCON_DECLARE(_name, compat, fn)                 \
    static const earlycon_id_t __UNIQUE_ID(__earlycon_##_name) \
        USED SECTION(".earlycon_table")                        \
            ALIGN(__alignof__(earlycon_id_t)) =                \
                {                                              \
                    .name = STRINGIFY(_name),                  \
                    .compatible = compat,                      \
                    .setup = fn,                               \
    }

#define EARLYCON_DECLARE(_name, fn) OF_EARLYCON_DECLARE(_name, "", fn)

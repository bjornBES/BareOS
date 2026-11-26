/*
 * File: kernel_levels.h
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>

#define ANSI_ESCAPE                    "\033"

#define ANSI_BOLD                      "1"
#define ANSI_FAINT                     "2"

#define ANSI_RESET                      ANSI_ESCAPE "[0m"

#define ANSI_FOREGROUND_COLOR(r, g, b) "38;2;" STRINGIFY(r) ";" STRINGIFY(g) ";" STRINGIFY(b)
#define ANSI_BACKGROUND_COLOR(r, g, b) "48;2;" STRINGIFY(r) ";" STRINGIFY(g) ";" STRINGIFY(b)

/* system is unusable */
#define KERNEL_EMERG_NAME              "EMERGENCY"
#define KERNEL_EMERG                   ANSI_ESCAPE "[4;" ANSI_BOLD ";" ANSI_BACKGROUND_COLOR(128, 0, 0) ";" ANSI_FOREGROUND_COLOR(255, 0, 0) "m"

/* action must be taken immediately */
#define KERNEL_ALERT_NAME              "ALERT"
#define KERNEL_ALERT                   ANSI_ESCAPE "[" ANSI_BOLD ";37;" ANSI_FOREGROUND_COLOR(255, 0, 0) "m"

/* critical conditions */
#define KERNEL_CRIT_NAME               "CRITICAL"
#define KERNEL_CRIT                    ANSI_ESCAPE "[" ANSI_BOLD ";37;31m"

/* error conditions */
#define KERNEL_ERR_NAME                "ERROR"
#define KERNEL_ERR                     ANSI_ESCAPE "[" ANSI_BOLD ";31m"

/* warning conditions */
#define KERNEL_WARNING_NAME            "WARNING"
#define KERNEL_WARNING                 ANSI_ESCAPE "[" ANSI_BOLD ";33m"

/* normal but significant condition */
#define KERNEL_NOTICE_NAME             "NOTICE"
#define KERNEL_NOTICE                  ANSI_ESCAPE "[" ANSI_BOLD ";37m"

/* informational */
#define KERNEL_INFO_NAME               "INFO"
#define KERNEL_INFO                    ANSI_ESCAPE "[37m"

/* debug-level messages */
#define KERNEL_DEBUG_NAME              "DEBUG"
#define KERNEL_DEBUG                   ANSI_ESCAPE "[" ANSI_FAINT ";37m"

/*
 * File: vt.h
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vtt.h"

#include <types.h>

status_t vty_init();

status_t vc_resize(virtual_console_data_t *vc, uint32_t cols, uint32_t rows, bool from_user);
status_t vc_take_over_console(const virt_con_ops_t *con, uint32_t first, uint32_t last, bool is_default);
status_t vc_register_con_driver(const virt_con_ops_t *con, uint32_t first, uint32_t last);

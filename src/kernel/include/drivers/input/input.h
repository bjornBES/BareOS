/*
 * File: input.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "input_dev.h"

#include <types.h>

status_t input_report_key(input_dev_t *input_dev, key_event_t *key);
input_dev_t *input_create_device();
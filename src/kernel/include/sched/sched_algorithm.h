/*
 * File: sched_algorithm.h
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "thread/threadt.h"
#include "lists/list.h"

#include "sched_class.h"

#include <types.h>
#include <config.h>

status_t sched_algorithm_get_algorithm(sched_class_t algorithms[CONFIG_MAX_SCHED_ALGORITHMS], int *out_index);

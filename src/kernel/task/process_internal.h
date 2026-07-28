/*
 * File: process_internal.h
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "process_types.h"

process_t *process_find_child(process_t *parent, pid_t child_pid);
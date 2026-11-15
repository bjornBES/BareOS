/*
 * File: cmdline.h
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "fat/fat.h"

#include "partition/partition.h"

#include <boot/params.h>

int cmdline_early_parse(partition_t *part, fat_file_t *file, char **out_kernel_path, boot_params_t *boot_params);
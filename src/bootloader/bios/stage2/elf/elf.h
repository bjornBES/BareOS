/*
 * File: elf.h
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <boot/params.h>
#include "partition/partition.h"

int elf_read_kernel(partition_t *part, boot_params_t *bp);

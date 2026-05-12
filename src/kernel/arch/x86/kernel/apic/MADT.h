/*
 * File: MADT.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "apic_type.h"
#include "madt_type.h"
#include "kernel/ACPI/ACPI.h"
#include "kernel/smp/cpu_config.h"

struct apic_priv;

void MADT_parse_madt(struct apic_priv *priv);
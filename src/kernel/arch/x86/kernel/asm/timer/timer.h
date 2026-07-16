/*
 * File: timer.h
 * File Created: 09 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once


#define TIMER_RATING_TSC        100     // best — if stable
#define TIMER_RATING_HPET        80     // stable, wide counter
#define TIMER_RATING_LAPIC       70     // per CPU, good for events
#define TIMER_RATING_ACPI_PM     50     // slow but reliable
#define TIMER_RATING_PIT         10     // last resort

/*
 * File: time.h
 * File Created: 09 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "x86.h"
#include "bios/bios.h"
#include <types.h>

#define CURRENT_YEAR 2026 // Change this each year!

static inline uint8_t bcd_to_bin(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

static int is_leap(int y)
{
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int days_in_month(int m, int y)
{
    static const int dim[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && is_leap(y)) ? 29 : dim[m - 1];
}

static inline time_t bcd_time_to_unix(uint8_t century, uint8_t year_bcd, uint8_t month_bcd, uint8_t day_bcd, uint8_t hour_bcd, uint8_t min_bcd, uint8_t sec_bcd)
{
    int year = bcd_to_bin(century ? century : 0x20) * 100 + bcd_to_bin(year_bcd);
    int month = bcd_to_bin(month_bcd);
    int day = bcd_to_bin(day_bcd);
    int hour = bcd_to_bin(hour_bcd);
    int min = bcd_to_bin(min_bcd);
    int sec = bcd_to_bin(sec_bcd);

    // Days from 1970 up to Jan 1 of this year
    long days = 0;
    for (int y = 1970; y < year; y++)
    {
        days += is_leap(y) ? 366 : 365;
    }

    // Days in completed months this year
    for (int m = 1; m < month; m++)
    {
        days += days_in_month(m, year);
    }

    days += (day - 1); // days into current month (0-indexed)

    return (time_t)days * 86400 + (time_t)hour * 3600 + (time_t)min * 60 + (time_t)sec;
}
static inline time_t time_to_unix(int year, int month, int day, int hour, int min, int sec)
{
    // Days from 1970 up to Jan 1 of this year
    long days = 0;
    for (int y = 1970; y < year; y++)
    {
        days += is_leap(y) ? 366 : 365;
    }

    // Days in completed months this year
    for (int m = 1; m < month; m++)
    {
        days += days_in_month(m, year);
    }

    days += (day - 1); // days into current month (0-indexed)

    return (time_t)days * 86400 + (time_t)hour * 3600 + (time_t)min * 60 + (time_t)sec;
}

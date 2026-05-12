/*
 * File: CMOS.c
 * File Created: 29 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "CMOS.h"

#include "libs/IO.h"

#include <stdbool.h>

#define CURRENT_YEAR 2026 // Change this each year!

#define CMOS_REGISTER_SEL 0x70
#define CMOS_DATA 0x71

#define CMOS_REG_SECONDS 0x00
#define CMOS_REG_SECONDS_ALARM 0x01
#define CMOS_REG_MINUTES 0x02
#define CMOS_REG_MINUTES_ALARM 0x03
#define CMOS_REG_HOURS 0x04
#define CMOS_REG_HOURS_ALARM 0x05
#define CMOS_REG_DAY_OF_WEEK 0x06
#define CMOS_REG_DATE_OF_MONTH 0x07
#define CMOS_REG_MONTH 0x08
#define CMOS_REG_YEAR 0x09
#define CMOS_REG_STATUSA 0x0A
#define CMOS_REG_STATUSB 0x0B
#define CMOS_REG_STATUSC 0x0C
#define CMOS_REG_STATUSD 0x0D

#define CMOS_REG_CENTURY 0x32

int century_register = 0x00;

#define FROM_BCD(val) ((val / 16) * 10 + (val & 0xf))

static inline void CMOS_write_register(uint8_t reg, uint8_t data)
{
    uint8_t nmi = inb(CMOS_REGISTER_SEL) & ~0x80;
    outb(CMOS_REGISTER_SEL, nmi | reg);
    outb(CMOS_DATA, data);
}
static inline uint8_t CMOS_read_register(uint8_t reg)
{
    uint8_t nmi = inb(CMOS_REGISTER_SEL) & ~0x80;
    outb(CMOS_REGISTER_SEL, nmi | reg);
    return inb(CMOS_DATA);
}

static inline bool is_progress_flag()
{
    return (CMOS_read_register(CMOS_REG_STATUSA) &0x80);
}

static inline int is_leap_year(int year)
{
    if (year % 4 == 0)
        return 1;
    return 0;
}

time_t CMOS_read_time()
{
    time_t result;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
    uint8_t century;

    // Wait until rtc is not updating
    while (is_progress_flag())
    {
    }

    second = CMOS_read_register(CMOS_REG_SECONDS);
    minute = CMOS_read_register(CMOS_REG_MINUTES);
    hour = CMOS_read_register(CMOS_REG_HOURS);
    day = CMOS_read_register(CMOS_REG_DATE_OF_MONTH);
    month = CMOS_read_register(CMOS_REG_MONTH);
    year = CMOS_read_register(CMOS_REG_YEAR);
    if (century_register != 0)
    {
        century = CMOS_read_register(century_register);
    }

    uint8_t registerB = CMOS_read_register(CMOS_REG_STATUSB);

    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04))
    {
        second = FROM_BCD(second);
        minute = FROM_BCD(minute);
        hour = FROM_BCD(hour) | (hour & 0x80);
        day = FROM_BCD(day);
        month = FROM_BCD(month);
        year = FROM_BCD(year);
        if (century_register != 0)
        {
            century = FROM_BCD(century);
        }
    }

    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(registerB & 0x02) && (hour & 0x80))
    {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year

    if (century_register != 0)
    {
        year += century * 100;
    }
    else
    {
        year += (CURRENT_YEAR / 100) * 100;
        if (year < CURRENT_YEAR)
            year += 100;
    }

    uint8_t month_code_array[] = {31, 28 + is_leap_year(year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // days from 1970 up to but not including current year
    uint32_t days = 0;
    for (uint32_t y = 1970; y < year; y++)
    {
        days += is_leap_year(y) ? 366 : 365;
    }

    // days elapsed in current year up to but not including current month
    for (uint8_t m = 1; m < month; m++)
    {
        days += month_code_array[m - 1];
    }

    // day of month (1-based, so subtract 1)
    days += day - 1;

    result = (time_t)days * 86400 + (time_t)hour * 3600 + (time_t)minute * 60 + (time_t)second;

    return result;
}
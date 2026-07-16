/*
 * File: bp_stage.h
 * File Created: 09 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

typedef enum
{
    USE_64_BIT_MODE,
    USE_32_BIT_MODE,
} bit_mode_t;


typedef struct bp_stage
{
    uint32_t mode;        //0
} __attribute__((packed)) bp_stage_t;

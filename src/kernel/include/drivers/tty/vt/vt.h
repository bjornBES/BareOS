/*
 * File: vt.h
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vtt.h"

#include <defs.h>
#include <types.h>

#define CREATE_VC_FLAG_ACCESSORS(accessor_name, flag_name)                  \
    ALWAYS_INLINE bool vc_##accessor_name(vc_data_t *vc)                    \
    {                                                                       \
        return BIT_GET(vc->vc_sw->con_flags, flag_name) == 1;               \
    }                                                                       \
    ALWAYS_INLINE void vc_set_##accessor_name(vc_data_t *vc)                \
    {                                                                       \
        BIT_SET(vc->vc_sw->con_flags, flag_name);                           \
    }                                                                       \
    ALWAYS_INLINE void vc_clear_##accessor_name(vc_data_t *vc)              \
    {                                                                       \
        BIT_UNSET(vc->vc_sw->con_flags, flag_name);                         \
    }                                                                       \
    ALWAYS_INLINE void vc_assign_##accessor_name(vc_data_t *vc, bool value) \
    {                                                                       \
        BIT_ASSIGN(vc->vc_sw->con_flags, flag_name, value);                 \
    }                                                                       \
    ALWAYS_INLINE bool vc_test_and_set_##accessor_name(vc_data_t *vc)       \
    {                                                                       \
        return BIT_TEST_AND_SET(vc->vc_sw->con_flags, flag_name);           \
    }

CREATE_VC_FLAG_ACCESSORS(use_utf8, VC_FLAGS_UTF8);
CREATE_VC_FLAG_ACCESSORS(can_color, VC_FLAGS_COLOR);
CREATE_VC_FLAG_ACCESSORS(use_text, VC_FLAGS_TEXT);

#undef CREATE_VC_FLAG_ACCESSORS

#define CREATE_VC_MODE_ACCESSORS(accessor_name, flag_name)                  \
    ALWAYS_INLINE bool vc_##accessor_name(vc_data_t *vc)                    \
    {                                                                       \
        return BIT_GET(vc->vc_modes, flag_name) == 1;                       \
    }                                                                       \
    ALWAYS_INLINE void vc_set_##accessor_name(vc_data_t *vc)                \
    {                                                                       \
        BIT_SET(vc->vc_modes, flag_name);                                   \
    }                                                                       \
    ALWAYS_INLINE void vc_clear_##accessor_name(vc_data_t *vc)              \
    {                                                                       \
        BIT_UNSET(vc->vc_modes, flag_name);                                 \
    }                                                                       \
    ALWAYS_INLINE void vc_assign_##accessor_name(vc_data_t *vc, bool value) \
    {                                                                       \
        BIT_ASSIGN(vc->vc_modes, flag_name, value);                         \
    }                                                                       \
    ALWAYS_INLINE bool vc_test_and_set_##accessor_name(vc_data_t *vc)       \
    {                                                                       \
        return BIT_TEST_AND_SET(vc->vc_modes, flag_name);                   \
    }

CREATE_VC_MODE_ACCESSORS(screen_mode, VC_MODE_SCREEN_MODE);
CREATE_VC_MODE_ACCESSORS(origin_mode, VC_MODE_ORIGIN_MODE);
CREATE_VC_MODE_ACCESSORS(autowrap_mode, VC_MODE_AUTOWRAP_MODE);
CREATE_VC_MODE_ACCESSORS(cursor_visible, VC_MODE_CUR_VISIBLE);
CREATE_VC_MODE_ACCESSORS(insert_mode, VC_MODE_INSERT_MODE);
CREATE_VC_MODE_ACCESSORS(need_wrap, VC_MODE_NEED_WRAP);

CREATE_VC_MODE_ACCESSORS(decscnm_mode, VC_MODE_SCREEN_MODE);
CREATE_VC_MODE_ACCESSORS(decom_mode, VC_MODE_ORIGIN_MODE);
CREATE_VC_MODE_ACCESSORS(decawm_mode, VC_MODE_AUTOWRAP_MODE);
CREATE_VC_MODE_ACCESSORS(deccm_visible, VC_MODE_CUR_VISIBLE);
CREATE_VC_MODE_ACCESSORS(decim_mode, VC_MODE_INSERT_MODE);

#undef CREATE_VC_MODE_ACCESSORS

status_t vty_init();

status_t vc_resize(vc_data_t *vc, uint32_t cols, uint32_t rows, bool from_user);
status_t vc_take_over_console(const virt_con_ops_t *con, uint32_t first, uint32_t last, bool is_default);
status_t vc_register_con_driver(const virt_con_ops_t *con, uint32_t first, uint32_t last);

/*
 * File: console_ringbuf.h
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "console_ringbuft.h"

#include "debug/debug.h"

#include "consolet.h"

#include <types.h>

#define CONSOLE_MESSAGE_MAX        2048

#define CONSOLE_RINGBUF_RECORD_MAX 1024

#define _DATA_SIZE(sz_bits)        (1ul << (sz_bits))
#define _DESCS_COUNT(ct_bits)      (1ul << (ct_bits))
#define DESC_SV_BITS               BITS_PER_LONG
#define DESC_FLAGS_SHIFT           (DESC_SV_BITS - 2)
#define DESC_FLAGS_MASK            (3ul << DESC_FLAGS_SHIFT)
#define DESC_STATE(sv)             (3ul & (sv >> DESC_FLAGS_SHIFT))
#define DESC_SV(id, state)         (((unsigned long)state << DESC_FLAGS_SHIFT) | id)
#define DESC_ID_MASK               (~DESC_FLAGS_MASK)
#define DESC_ID(sv)                ((sv) & DESC_ID_MASK)

#define FAILED_POS                 0x1
#define EMPTY_LINE_POS             0x3

#define FAILED_BLK_POS       \
    {                        \
        .begin = FAILED_POS, \
        .next = FAILED_POS,  \
    }

#define BLK0_POS(sz_bits) (-(_DATA_SIZE(sz_bits)))
#define DESC0_ID(ct_bits) DESC_ID(-(_DESCS_COUNT(ct_bits) + 1))
#define DESC0_SV(ct_bits) DESC_SV(DESC0_ID(ct_bits), descriptor_reusable)


static inline void cons_ringbuf_rec_init_wr(cons_record_t *record, size_t text_buf_size)
{
	record->info = NULL;
	record->text_buf = NULL;
	record->text_buf_size = text_buf_size;
}

static inline void cons_ringbuf_rec_init_rd(cons_record_t *record, message_info_t *info, char *text_buf, size_t text_buf_size)
{
	record->info = info;
	record->text_buf = text_buf;
	record->text_buf_size = text_buf_size;
}

status_t cons_ringbuf_reserve(cons_ringbuf_t *ringbuf, cons_ringbuf_resv_entry_t *resv_entry, cons_record_t *record);

void cons_ringbuf_commit(cons_ringbuf_resv_entry_t *e);
void cons_ringbuf_final_commit(cons_ringbuf_resv_entry_t *e);

bool cons_ringbuf_read_valid(cons_ringbuf_t *ringbuf, uint64_t seq, cons_record_t *r);

uint64_t cons_ringbuf_first_seq(cons_ringbuf_t *ringbuf);
uint64_t cons_ringbuf_next_reserve_seq(cons_ringbuf_t *ringbuf);


#define __u64seq_to_ulseq(u64seq) ((uint32_t)u64seq)

static inline uint64_t __ulseq_to_u64seq(cons_ringbuf_t *ringbuf, uint32_t ulseq)
{
	uint64_t rb_first_seq = cons_ringbuf_first_seq(ringbuf);
	uint64_t seq;

	seq = rb_first_seq - (int32_t)((uint32_t)rb_first_seq - ulseq);

	return seq;
}

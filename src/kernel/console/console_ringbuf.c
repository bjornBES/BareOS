/*
 * File: console_ringbuf.c
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "console/console_ringbuf.h"

#include "debug/debug.h"
#include "kerrno.h"

#include "math.h"
#include "memory.h"

#include <defs.h>

#define MODULE                                     "cons-ringbuf"

#define DATA_SIZE(data_ring)                       _DATA_SIZE((data_ring)->size_bits)
#define DATA_SIZE_MASK(data_ring)                  (DATA_SIZE(data_ring) - 1)

#define DESCS_COUNT(desc_ring)                     _DESCS_COUNT((desc_ring)->count_bits)
#define DESCS_COUNT_MASK(desc_ring)                (DESCS_COUNT(desc_ring) - 1)

#define DATA_INDEX(data_ring, lpos)                ((lpos) & DATA_SIZE_MASK(data_ring))

#define DESC_INDEX(desc_ring, n)                   ((n) & DESCS_COUNT_MASK(desc_ring))

#define DATA_WRAPS(data_ring, pos)                 ((pos) >> (data_ring)->size_bits)

#define POS_DATALESS(lpos)                         ((lpos) & 1ul)
#define BLK_DATALESS(blk)                          (POS_DATALESS((blk)->begin) && POS_DATALESS((blk)->next))

#define DATA_THIS_WRAP_START_LPOS(data_ring, lpos) ((lpos) & ~DATA_SIZE_MASK(data_ring))

#define DESC_ID_PREV_WRAP(desc_ring, id)           DESC_ID((id) - DESCS_COUNT(desc_ring))

#define LOCK_NAME                                  console_ringbuf_lock
DEF_LOCK(LOCK_NAME);

INTERNAL cons_ringbuf_desc_t *to_desc(cons_ringbuf_desc_ring_t *desc_ring, uint64_t n)
{
    return &desc_ring->descs[DESC_INDEX(desc_ring, n)];
}

INTERNAL message_info_t *to_info(cons_ringbuf_desc_ring_t *desc_ring, uint64_t n)
{
    return &desc_ring->infos[DESC_INDEX(desc_ring, n)];
}

static cons_ringbuf_data_block_t *to_block(cons_ringbuf_data_ring_t *data_ring, uint32_64_t begin_lpos)
{
    return (void *)&data_ring->data[DATA_INDEX(data_ring, begin_lpos)];
}

INTERNAL bool need_more_space(cons_ringbuf_data_ring_t *data_ring, uint32_64_t lpos_current, uint32_64_t lpos_target)
{
    return lpos_target - lpos_current - 1 < DATA_SIZE(data_ring);
}

INTERNAL descriptor_state_t get_desc_state(uint32_64_t id, uint32_64_t state_val)
{
    if (id != DESC_ID(state_val))
    {
        return descriptor_miss;
    }

    return DESC_STATE(state_val);
}

INTERNAL size_t to_blk_size(size_t size)
{
    size += sizeof(cons_ringbuf_data_block_t);
    size = ALIGN_2_DOWN(size, sizeof(uint32_64_t));
    return size;
}

INTERNAL bool data_check_size(cons_ringbuf_data_ring_t *data_ring, size_t size)
{
    if (size == 0)
    {
        return true;
    }

    return to_blk_size(size) <= DATA_SIZE(data_ring) / 2;
}

INTERNAL descriptor_state_t desc_read(cons_ringbuf_desc_ring_t *desc_ring, uint32_64_t id, cons_ringbuf_desc_t *desc_out, uint64_t *seq_out, message_caller_id_t *caller_id_out)
{
    // log_info(MODULE, "desc_read(%p, 0x%llx, %p, %p, %p)", desc_ring, id, desc_out, seq_out, caller_id_out);
    message_info_t *info = to_info(desc_ring, id);
    cons_ringbuf_desc_t *desc = to_desc(desc_ring, id);
    uint32_64_t state_val = desc->state_var;

    descriptor_state_t d_state = get_desc_state(id, state_val);
    if (d_state == descriptor_miss || d_state == descriptor_reserved)
    {
        goto out;
    }

    // smp_rmb();

    if (desc_out)
    {
        memcpy(&desc_out->text_blk_pos, &desc->text_blk_pos, sizeof(cons_ringbuf_data_block_pos_t));
    }
    if (seq_out)
    {
        *seq_out = info->seq;
    }
    if (caller_id_out)
    {
        *caller_id_out = info->caller_id;
    }

    // smp_rmb();

    state_val = desc->state_var;
    d_state = get_desc_state(id, state_val);
out:
    if (desc_out)
    {
        desc_out->state_var = state_val;
    }
    return d_state;
}

INTERNAL void desc_make_reusable(cons_ringbuf_desc_ring_t *desc_ring, uint32_64_t id)
{
    log_err(MODULE, "desc_make_reusable(%p, %p)", desc_ring, id);
    uint32_64_t val_finalized = DESC_SV(id, descriptor_finalized);
    uint32_64_t val_reusable = DESC_SV(id, descriptor_reusable);
    cons_ringbuf_desc_t *desc = to_desc(desc_ring, id);

    if (desc->state_var == val_finalized)
    {
        desc->state_var = val_reusable;
    }
}

INTERNAL bool data_make_reusable(cons_ringbuf_t *ringbuf, uint32_64_t pos_begin, uint32_64_t lpos_end, uint32_64_t *pos_out)
{

    cons_ringbuf_data_ring_t *data_ring = &ringbuf->text_data_ring;
    cons_ringbuf_desc_ring_t *desc_ring = &ringbuf->desc_ring;
    cons_ringbuf_desc_t desc;
    cons_ringbuf_data_block_pos_t *blk_pos = &desc.text_blk_pos;

    while (need_more_space(data_ring, pos_begin, lpos_end))
    {
        cons_ringbuf_data_block_t *blk = to_block(data_ring, pos_begin);
        uint32_64_t id = blk->id;

        descriptor_state_t d_state = desc_read(desc_ring, id, &desc, NULL, NULL);

        switch (d_state)
        {
            case descriptor_miss :
            case descriptor_reserved :
            case descriptor_committed :
                return false;
            case descriptor_finalized :
                if (blk_pos->begin != pos_begin)
                {
                    return false;
                }
                desc_make_reusable(desc_ring, id);
                break;
            case descriptor_reusable :
                if (blk_pos->begin != pos_begin)
                {
                    return false;
                }
                break;
        }

        pos_begin = blk_pos->next;
    }

    *pos_out = pos_begin;
    return true;
}

INTERNAL bool data_push_tail(cons_ringbuf_t *ringbuf, cons_ringbuf_data_ring_t *data_ring, uint32_64_t pos)
{
    uint32_64_t tail_pos_new;
    uint32_64_t tail_pos;
    uint32_64_t next_pos;

    if (POS_DATALESS(pos))
    {
        return true;
    }

    tail_pos = data_ring->tail_pos;

    while (need_more_space(data_ring, tail_pos, pos))
    {
        if (!data_make_reusable(ringbuf, tail_pos, pos, &next_pos))
        {
            // smp_rmb();

            tail_pos_new = data_ring->tail_pos;
            if (tail_pos_new == tail_pos)
            {
                return false;
            }

            tail_pos = tail_pos_new;
            continue;
        }

        if (data_ring->tail_pos == tail_pos)
        {
            data_ring->tail_pos = next_pos;
            break;
        }
    }

    return true;
}

INTERNAL status_t desc_reserve(cons_ringbuf_t *ringbuf, uint32_64_t *out_id)
{
    // log_info(MODULE, "desc_reserve(%p, %p)", ringbuf, out_id);
    cons_ringbuf_desc_ring_t *desc_ring = &ringbuf->desc_ring;
    uint32_64_t head_id = desc_ring->head_id;

    uint32_64_t id = 0;
    uint32_64_t id_prev_wrap = 0;
    while (true)
    {

        id = DESC_ID(head_id + 1);
        id_prev_wrap = DESC_ID_PREV_WRAP(desc_ring, id);

        if (id_prev_wrap == desc_ring->tail_id)
        {
            FUNC_NOT_IMPLEMENTED();
        }

        if (desc_ring->head_id == head_id)
        {
            desc_ring->head_id = id;
            break;
        }
    }

    cons_ringbuf_desc_t *desc = to_desc(desc_ring, id);

    uint32_64_t prev_state_val = desc->state_var;
    if (prev_state_val && get_desc_state(id_prev_wrap, prev_state_val) != descriptor_reusable)
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_UNSUCCESS, "");
    }

    if (desc->state_var == prev_state_val)
    {
        desc->state_var = DESC_SV(id, descriptor_reserved);
    }
    else
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_UNSUCCESS, "");
    }

    *out_id = id;
    return KERRNO_SUCCESS;
}

INTERNAL bool is_blk_wrapped(cons_ringbuf_data_ring_t *data_ring, uint32_64_t begin_lpos, uint32_64_t next_lpos)
{
    return DATA_WRAPS(data_ring, begin_lpos) != DATA_WRAPS(data_ring, next_lpos - 1);
}

INTERNAL uint32_64_t get_next_lpos(cons_ringbuf_data_ring_t *data_ring, uint32_64_t lpos, size_t size)
{
    uint32_64_t begin_lpos;
    uint32_64_t next_lpos;

    begin_lpos = lpos;
    next_lpos = lpos + size;

    /* First check if the data block does not wrap. */
    if (!is_blk_wrapped(data_ring, begin_lpos, next_lpos))
    {
        return next_lpos;
    }

    /* Wrapping data blocks store their data at the beginning. */
    return DATA_THIS_WRAP_START_LPOS(data_ring, next_lpos) + size;
}

INTERNAL status_t data_alloc(cons_ringbuf_t *ringbuf, size_t size, cons_ringbuf_data_block_pos_t *blk_pos, uint32_64_t id, char **out)
{
    // log_info(MODULE, "data_alloc(%p, %p, %p, %p, %p)", ringbuf, size, blk_pos, id, out);
    cons_ringbuf_data_ring_t *data_ring = &ringbuf->text_data_ring;
    cons_ringbuf_data_block_t *blk;
    uint32_64_t begin_pos;
    uint32_64_t next_pos;

    if (size == 0)
    {
        blk_pos->begin = EMPTY_LINE_POS;
        blk_pos->next = EMPTY_LINE_POS;
        out = NULL;
        KERRNO_RETURN_NO_PRINT(KERRNO_UNSUCCESS, "");
    }

    size = to_blk_size(size);

    begin_pos = data_ring->head_pos;

    do
    {
        next_pos = get_next_lpos(data_ring, begin_pos, size);

        if (WARN_ON_ONCE(next_pos - begin_pos > DATA_SIZE(data_ring)) || !data_push_tail(ringbuf, data_ring, next_pos - DATA_SIZE(data_ring)))
        {
            blk_pos->begin = FAILED_POS;
            blk_pos->next = FAILED_POS;
            out = NULL;
            KERRNO_RETURN_NO_PRINT(KERRNO_UNSUCCESS, "");
        }

        if (data_ring->head_pos == begin_pos)
        {
            data_ring->head_pos = next_pos;
            break;
        }
    } while (true);

    blk = to_block(data_ring, begin_pos);
    blk->id = id;

    if (is_blk_wrapped(data_ring, begin_pos, next_pos))
    {
        blk = to_block(data_ring, 0);

        blk->id = id;
    }

    blk_pos->begin = begin_pos;
    blk_pos->next = next_pos;

    *out = blk->data;
    return KERRNO_SUCCESS;
}

INTERNAL uint32_t space_used(cons_ringbuf_data_ring_t *data_ring, cons_ringbuf_data_block_pos_t *blk_lpos)
{
    // log_info(MODULE, "space_used(%p, %p)", data_ring, blk_lpos);
    if (BLK_DATALESS(blk_lpos))
    {
        return 0;
    }

    if (!is_blk_wrapped(data_ring, blk_lpos->begin, blk_lpos->next))
    {
        return DATA_INDEX(data_ring, blk_lpos->next) - DATA_INDEX(data_ring, blk_lpos->begin);
    }

    return DATA_INDEX(data_ring, blk_lpos->next) + DATA_SIZE(data_ring) - DATA_INDEX(data_ring, blk_lpos->begin);
}

INTERNAL bool data_ring_size(cons_ringbuf_data_ring_t *data_ring, size_t size)
{
    if (size == 0)
    {
        return true;
    }

    return to_blk_size(size) <= DATA_SIZE(data_ring) / 2;
}

INTERNAL const char *get_data(cons_ringbuf_data_ring_t *data_ring, cons_ringbuf_data_block_pos_t *blk_lpos, size_t *data_size)
{
    cons_ringbuf_data_block_t *db;

    if (BLK_DATALESS(blk_lpos))
    {
        if (blk_lpos->begin == EMPTY_LINE_POS && blk_lpos->next == EMPTY_LINE_POS)
        {
            *data_size = 0;
            return "";
        }

        return NULL;
    }

    if (!is_blk_wrapped(data_ring, blk_lpos->begin, blk_lpos->next))
    {
        db = to_block(data_ring, blk_lpos->begin);
        *data_size = blk_lpos->next - blk_lpos->begin;
    }
    else if (!is_blk_wrapped(data_ring, blk_lpos->begin + DATA_SIZE(data_ring), blk_lpos->next))
    {
        db = to_block(data_ring, 0);
        *data_size = DATA_INDEX(data_ring, blk_lpos->next);
    }
    else
    {
        return NULL;
    }

    if (blk_lpos->begin != ALIGN_DOWN(blk_lpos->begin, sizeof(db->id)) || blk_lpos->next != ALIGN_DOWN(blk_lpos->next, sizeof(db->id)))
    {
        return NULL;
    }

    if (*data_size <= sizeof(db->id))
    {
        return NULL;
    }

    *data_size -= sizeof(db->id);

    if (!data_check_size(data_ring, *data_size))
    {
        return NULL;
    }

    return &db->data[0];
}

INTERNAL uint64_t desc_last_finalized_seq(cons_ringbuf_t *ringbuf, cons_ringbuf_desc_ring_t *desc_ring)
{
    // log_info(MODULE, "desc_last_finalized_seq(%p, %p)", ringbuf, desc_ring);
    uint32_64_t ulseq = desc_ring->last_finalized_seq;

    return __ulseq_to_u64seq(ringbuf, ulseq);
}

INTERNAL bool _cons_ringbuf_read_valid(cons_ringbuf_t *ringbuf, uint64_t *seq, cons_record_t *r, unsigned int *line_count);

INTERNAL void desc_update_last_finalized(cons_ringbuf_t *ringbuf, cons_ringbuf_desc_ring_t *desc_ring)
{
    // log_info(MODULE, "desc_update_last_finalized(%p, %p)", ringbuf, desc_ring);
    uint32_64_t old_seq = desc_last_finalized_seq(ringbuf, desc_ring);

try_again:
    uint32_64_t finalized_seq = old_seq;
    uint64_t try_seq = finalized_seq + 1;

    while (_cons_ringbuf_read_valid(ringbuf, &try_seq, NULL, NULL))
    {
        finalized_seq = try_seq;
        try_seq++;
    }

    if (finalized_seq == old_seq)
    {
        // LOG_RETURN_NO();
        return;
    }

    uint64_t oldval = __u64seq_to_ulseq(old_seq);
    uint64_t newval = __u64seq_to_ulseq(finalized_seq);

    if (desc_ring->last_finalized_seq == oldval)
    {
        desc_ring->last_finalized_seq = newval;
    }
    else
    {
        old_seq = __ulseq_to_u64seq(ringbuf, oldval);
        goto try_again;
    }
}

INTERNAL void desc_make_final(cons_ringbuf_t *ringbuf, uint32_64_t id)
{
    // log_info(MODULE, "desc_make_final(%p, %p)", ringbuf, id);
    cons_ringbuf_desc_ring_t *desc_ring = &ringbuf->desc_ring;
    uint32_64_t prev_state_val = DESC_SV(id, descriptor_committed);
    cons_ringbuf_desc_t *d = to_desc(desc_ring, id);

    if (d->state_var == prev_state_val)
    {
        d->state_var = DESC_SV(id, descriptor_finalized);
        desc_update_last_finalized(ringbuf, desc_ring);
    }
}

status_t cons_ringbuf_reserve(cons_ringbuf_t *ringbuf, cons_ringbuf_resv_entry_t *resv_entry, cons_record_t *record)
{
    if (!data_ring_size(&ringbuf->text_data_ring, record->text_buf_size))
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_BAD_VALUE, "");
    }

    LOCK();

    uint32_64_t id;
    status_t ret = desc_reserve(ringbuf, &id);
    if (ret)
    {
        ringbuf->fail++;
        UNLOCK();
        return ret;
    }

    cons_ringbuf_desc_ring_t *desc_ring = &ringbuf->desc_ring;

    cons_ringbuf_desc_t *desc = to_desc(desc_ring, id);
    message_info_t *info = to_info(desc_ring, id);

    uint64_t seq = info->seq;
    memset(info, 0, sizeof(*info));

    resv_entry->rb = ringbuf;
    resv_entry->id = id;

    if (seq == 0 && DESC_INDEX(desc_ring, id) != 0)
    {
        info->seq = DESC_INDEX(desc_ring, id);
    }
    else
    {
        info->seq = seq + DESCS_COUNT(desc_ring);
    }

    if (info->seq > 0)
    {
        desc_make_final(ringbuf, DESC_ID(id - 1));
    }

    ret = data_alloc(ringbuf, record->text_buf_size+8, &desc->text_blk_pos, id, &record->text_buf);

    if (ret)
    {
        UNLOCK();
        return ret;
    }

    if (record->text_buf_size && !record->text_buf)
    {
        cons_ringbuf_commit(resv_entry);
        UNLOCK();
        // LOG_RETURN(ret);
        return ret;
    }

    record->info = info;

    resv_entry->text_space = space_used(&ringbuf->text_data_ring, &desc->text_blk_pos);
    return KERRNO_SUCCESS;
}

INTERNAL void _cons_ringbuf_commit(cons_ringbuf_resv_entry_t *e, cons_ringbuf_desc_ring_t *desc_ring, uint32_64_t state_val)
{
    // log_info(MODULE, "_cons_ringbuf_commit(%p, %p, %u)", e, desc_ring, state_val);
    cons_ringbuf_desc_t *d = to_desc(desc_ring, e->id);
    uint32_64_t prev_state_val = DESC_SV(e->id, descriptor_reserved);

    if (d->state_var == prev_state_val)
    {
        d->state_var = DESC_SV(e->id, state_val);
    }
    else
    {
        WARN_ON_ONCE(1);
    }

    UNLOCK();
}

void cons_ringbuf_commit(cons_ringbuf_resv_entry_t *e)
{
    // log_info(MODULE, "cons_ringbuf_commit(%p)", e);
    cons_ringbuf_desc_ring_t *desc_ring = &e->rb->desc_ring;
    uint32_64_t head_id;

    _cons_ringbuf_commit(e, desc_ring, descriptor_committed);

    head_id = desc_ring->head_id;
    if (head_id != e->id)
    {
        desc_make_final(e->rb, e->id);
    }
}

void cons_ringbuf_final_commit(cons_ringbuf_resv_entry_t *e)
{
    // log_info(MODULE, "cons_ringbuf_final_commit(%p)", e);
    cons_ringbuf_desc_ring_t *desc_ring = &e->rb->desc_ring;
    _cons_ringbuf_commit(e, desc_ring, descriptor_finalized);

    desc_update_last_finalized(e->rb, desc_ring);
}

INTERNAL uint32_t count_lines(const char *text, uint32_t text_size)
{
    uint32_t next_size = text_size;
    uint32_t line_count = 1;
    const char *next = text;

    while (next_size)
    {
        next = memchr(next, '\n', next_size);
        if (!next)
        {
            break;
        }
        line_count++;
        next++;
        next_size = text_size - (next - text);
    }

    return line_count;
}

INTERNAL bool copy_data(cons_ringbuf_data_ring_t *data_ring, cons_ringbuf_data_block_pos_t *blk_lpos, uint16_t len, char *buf, uint32_t buf_size, uint32_t *line_count)
{
    size_t data_size;
    const char *data;

    if ((!buf || !buf_size) && !line_count)
    {
        return true;
    }

    data = get_data(data_ring, blk_lpos, &data_size);
    if (!data)
    {
        return false;
    }

    if (data_size < (size_t)len)
    {
        return false;
    }

    if (line_count)
    {
        *line_count = count_lines(data, len);
    }

    if (!buf || !buf_size)
    {
        return true;
    }

    data_size = (uint32_t)min(buf_size, len);

    memcpy(&buf[0], data, data_size);
    return true;
}

INTERNAL status_t desc_read_finalized_seq(cons_ringbuf_desc_ring_t *desc_ring, uint32_64_t id, uint64_t seq, cons_ringbuf_desc_t *desc_out)
{
    // log_info(MODULE, "desc_read_finalized_seq(%p, %p, %p, %p)", desc_ring, id, seq, desc_out);
    cons_ringbuf_data_block_pos_t *blk_lpos = &desc_out->text_blk_pos;
    descriptor_state_t d_state;
    uint64_t s;

    d_state = desc_read(desc_ring, id, desc_out, &s, NULL);

    if (d_state == descriptor_miss || d_state == descriptor_reserved || d_state == descriptor_committed || s != seq)
    {
        return KERRNO_BAD_VALUE;
        KERRNO_RETURN_NO_PRINT(KERRNO_BAD_VALUE, "")
    }

    if (d_state == descriptor_reusable || (blk_lpos->begin == FAILED_POS && blk_lpos->next == FAILED_POS))
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_ENTRY_NOT_FOUND, "")
    }

    return KERRNO_SUCCESS;
}

INTERNAL status_t cons_ringbuf_read(cons_ringbuf_t *ringbuf, cons_ringbuf_desc_ring_t *desc_ring, uint64_t seq, cons_record_t *r, unsigned int *line_count)
{
    // log_info(MODULE, "cons_ringbuf_read(%p, %p, %p, %p, %p)", ringbuf, desc_ring, seq, r, line_count);
    message_info_t *info = to_info(desc_ring, seq);
    cons_ringbuf_desc_t *rdesc = to_desc(desc_ring, seq);
    cons_ringbuf_desc_t desc;
    
    uint32_64_t id = DESC_ID(rdesc->state_var);
    
    status_t err = desc_read_finalized_seq(desc_ring, id, seq, &desc);

    if (err || !r)
    {
        // LOG_RETURN(err);
        return err;
    }

    if (r->info)
    {
        memcpy(r->info, info, sizeof(message_info_t));
    }

    if (!copy_data(&ringbuf->text_data_ring, &desc.text_blk_pos, info->text_len, r->text_buf, r->text_buf_size, line_count))
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_ENTRY_NOT_FOUND, "");
    }

    return desc_read_finalized_seq(desc_ring, id, seq, &desc);
}

uint64_t cons_ringbuf_first_seq(cons_ringbuf_t *ringbuf)
{
    // log_info(MODULE, "cons_ringbuf_first_seq(%p)", ringbuf);
    cons_ringbuf_desc_ring_t *desc_ring = &ringbuf->desc_ring;
    uint64_t seq;

    while (true)
    {
        uint32_64_t id = ringbuf->desc_ring.tail_id;

        cons_ringbuf_desc_t desc;
        descriptor_state_t d_state = desc_read(desc_ring, id, &desc, &seq, NULL);
        if (d_state == descriptor_finalized || d_state == descriptor_reusable)
        {
            break;
        }
        // smp_rmb();
    }

    return seq;
}

uint64_t cons_ringbuf_next_reserve_seq(cons_ringbuf_t *ringbuf)
{
    cons_ringbuf_desc_ring_t *desc_ring = &ringbuf->desc_ring;
    uint32_64_t last_finalized_id;
    cons_ringbuf_desc_t desc;
    uint32_64_t diff;
    cons_ringbuf_desc_t *d;
    status_t err;

try_again:
    uint64_t last_finalized_seq;
    last_finalized_seq = desc_last_finalized_seq(ringbuf, desc_ring);

    uint32_64_t head_id = desc_ring->head_id;

    d = to_desc(desc_ring, last_finalized_seq);

    last_finalized_id = DESC_ID(d->state_var);

    err = desc_read_finalized_seq(desc_ring, last_finalized_id, last_finalized_seq, &desc);

    if (err == KERRNO_BAD_VALUE)
    {
        if (last_finalized_seq == 0)
        {
            if (head_id == DESC0_ID(desc_ring->count_bits))
            {
                return 0;
            }

            last_finalized_id = DESC0_ID(desc_ring->count_bits) + 1;
        }
        else
        {
            goto try_again;
        }
    }

    diff = head_id - last_finalized_id;

    return last_finalized_seq + diff + 1;
}

INTERNAL bool _cons_ringbuf_read_valid(cons_ringbuf_t *ringbuf, uint64_t *seq, cons_record_t *r, unsigned int *line_count)
{
    // log_info(MODULE, "_cons_ringbuf_read_valid(%p, %p, %p, %p)", ringbuf, seq, r, line_count);
    uint64_t tail_id;
    status_t err;

    while ((err = cons_ringbuf_read(ringbuf, &ringbuf->desc_ring, *seq, r, line_count)))
    {
        tail_id = cons_ringbuf_first_seq(ringbuf);

        if (*seq < tail_id)
        {
            *seq = tail_id;
        }
        else if (err == KERRNO_ENTRY_NOT_FOUND)
        {
            (*seq)++;
        }
        else
        {
            if (panic_on_this_cpu() && ((*seq + 1) < cons_ringbuf_next_reserve_seq(ringbuf)))
            {
                (*seq)++;
            }
            else
            {
                // LOG_RETURN(false);
                return false;
            }
        }
    }
    
    // LOG_RETURN(true);
    return true;
}

bool cons_ringbuf_read_valid(cons_ringbuf_t *ringbuf, uint64_t seq, cons_record_t *r)
{
    return _cons_ringbuf_read_valid(ringbuf, &seq, r, NULL);
}

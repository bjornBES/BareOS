/*
 * File: console_ringbuft.h
 * File Created: 22 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "consolet.h"

#include "debug/trace.h"

#include <types.h>

typedef struct message_caller_id
{
    tid_t thread_id;
    cpu_logical_id_t cpu_id;
    pid_t process_id;
} message_caller_id_t;


typedef struct message_info
{
    timespec_t time; /* timestamp */
    uint64_t seq;    /* sequence number */

    size_t text_len; /* length of text message */

    message_caller_id_t caller_id;

    trace_level_t log_level;
    con_write_level_t level;

    uint8_t flags; /* internal record flags */
} message_info_t;

typedef struct cons_record
{
    message_info_t *info;
    char *text_buf;
    size_t text_buf_size;
} cons_record_t;

typedef struct cons_ringbuf_data_block_pos
{
    uint32_64_t begin;
    uint32_64_t next;
} cons_ringbuf_data_block_pos_t;

typedef struct cons_ringbuf_desc
{
    cons_ringbuf_data_block_pos_t text_blk_pos;
    uint32_64_t state_var;
} cons_ringbuf_desc_t;

typedef struct cons_ringbuf_data_block
{
    uint32_64_t id;
    char data[];
} cons_ringbuf_data_block_t;

typedef struct cons_ringbuf_data_ring
{
    char *data;
    uint32_64_t head_pos;
    uint32_64_t tail_pos;
    uint32_t size_bits;
} cons_ringbuf_data_ring_t;

typedef struct cons_ringbuf_desc_ring
{
    cons_ringbuf_desc_t *descs;
    message_info_t *infos;
    uint32_64_t head_id;
    uint32_64_t tail_id;
    uint32_64_t last_finalized_seq;
    uint32_t count_bits;
} cons_ringbuf_desc_ring_t;

typedef struct cons_ringbuf
{
    cons_ringbuf_desc_ring_t desc_ring;
    cons_ringbuf_data_ring_t text_data_ring;
    uint32_64_t fail;
} cons_ringbuf_t;

typedef struct cons_ringbuf_resv_entry
{
    cons_ringbuf_t *rb;
    uint32_64_t id;
    uint32_t text_space;
} cons_ringbuf_resv_entry_t;

typedef enum descriptor_state
{
    descriptor_miss = -1,       /* ID mismatch (pseudo state) */
    descriptor_reserved = 0x0,  /* reserved, in use by writer */
    descriptor_committed = 0x1, /* committed by writer, could get reopened */
    descriptor_finalized = 0x2, /* committed, no further modification allowed */
    descriptor_reusable = 0x3,  /* free, not yet used by any writer */
} descriptor_state_t;

/*
 * File: console.c
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "console/consolet.h"
#include "console/console.h"
#include "console/console_ringbuf.h"

#include "drivers/tty/tty.h"

#include "asm/cpu_arch.h"

#include "lists/list.h"

#include "sync/mutex.h"
#include "sync/semaphore.h"
#include "sync/spinlock.h"

#include "assert/assert.h"

#include "cmdline/cmdline.h"

#include "string.h"
#include "ctype.h"
#include "stdio.h"
#include "memory.h"

#include "kerrno.h"

#include "init.h"

#include <config.h>

#define MODULE "console"

list_t console_list;

#define console_first()           \
    to_console(console_list.head)

#define LOCK_NAME console_lock
DEF_LOCK(LOCK_NAME);

#define MAX_CMDLINECONSOLES 8

static bool is_console_cmdline_zeroed = false;
static console_cmdline_t console_cmdline[MAX_CMDLINECONSOLES];

bool have_boot_console = false;
bool have_real_console = false;

static int preferred_console = -1;
static int console_set_on_cmdline;

static int console_locked;
static int console_may_schedule;

mutex_t syslog_lock;
static uint64_t syslog_seq;

mutex_t console_mutex;
semaphore_t console_sem;

static int keep_bootcon;
static uint64_t sequence_number;

#define CONFIG_LOG_BUF_SHIFT 18
#define LOG_ALIGN            __alignof__(unsigned long)
#define __LOG_BUF_LEN        (1 << CONFIG_LOG_BUF_SHIFT)
#define LOG_BUF_LEN_MAX      ((uint32_t)1 << 31)
char __log_buf[__LOG_BUF_LEN] ALIGN(LOG_ALIGN);

#define _DEFINE_PRINTKRB(name, descbits, avgtextbits, text_buf)     \
    cons_ringbuf_desc_t _##name##_descs[_DESCS_COUNT(descbits)] = { \
        [_DESCS_COUNT(descbits) - 1] = {                            \
            .state_var = DESC0_SV(descbits),                        \
            .text_blk_pos = FAILED_BLK_POS,                         \
        },                                                          \
    };                                                              \
    message_info_t _##name##_infos[_DESCS_COUNT(descbits)] = {      \
        [0] = {                                                     \
            .seq = -(uint64_t)_DESCS_COUNT(descbits),               \
        },                                                          \
        [_DESCS_COUNT(descbits) - 1] = {                            \
            .seq = 0,                                               \
        },                                                          \
    };                                                              \
    cons_ringbuf_t name = {                                         \
        .desc_ring = {                                              \
            .count_bits = descbits,                                 \
            .descs = &_##name##_descs[0],                           \
            .infos = &_##name##_infos[0],                           \
            .head_id = DESC0_ID(descbits),                          \
            .tail_id = DESC0_ID(descbits),                          \
            .last_finalized_seq = 0,                                \
        },                                                          \
        .text_data_ring = {                                         \
            .size_bits = (avgtextbits) + (descbits),                \
            .data = text_buf,                                       \
            .head_pos = BLK0_POS((avgtextbits) + (descbits)),       \
            .tail_pos = BLK0_POS((avgtextbits) + (descbits)),       \
        },                                                          \
        .fail = 0,                                                  \
    }

#define PRB_AVGBITS 5 /* 32 character average length */

static uint32_t log_buf_len = __LOG_BUF_LEN;

_DEFINE_PRINTKRB(printk_rb_static, CONFIG_LOG_BUF_SHIFT - PRB_AVGBITS, PRB_AVGBITS, __log_buf);
cons_ringbuf_t *prb = &printk_rb_static;

con_write_level_t console_loglevel = CON_WRITE_LEVEL_DEBUG;

console_buffers_t shared_console_bufs;

#define MAX_LOG_TAKE_PART 4
static const char trunc_msg[] = "<truncated>";

static void truncate_msg(size_t *text_len, size_t *trunc_msg_len)
{
    uint32_t max_text_len = log_buf_len / MAX_LOG_TAKE_PART;

    if (*text_len > max_text_len)
    {
        *text_len = max_text_len;
    }

    *trunc_msg_len = strlen(trunc_msg);
    if (*text_len >= *trunc_msg_len)
    {
        *text_len -= *trunc_msg_len;
    }
    else
    {
        *trunc_msg_len = 0;
    }
}

#if CONFIG_PRINT_TIME == 1
static size_t print_time(timespec_t *ts, char *buf)
{
    uint64_t ns = ts->tv_nsec;
    if (ns == 0)
    {
#if CONFIG_PRINT_RAW_TIME == 0
        return sprintf(buf, "[00:00:00.000] ");
#else
        return sprintf(buf, "[0000.000000] ");
#endif
    }
    uint64_t sec = ts->tv_sec;
#if CONFIG_PRINT_RAW_TIME == 0
    uint32_t ms = NANOSEC_TO_MILLISEC(ts->tv_nsec);
    uint32_t min = sec / 60;
    uint32_t hour = min / 60;

    return sprintf(buf, "[%02u:%02u:%02u.%03u] ", hour % 24, min % 60, sec % 60, ms % 1000);
#else
    return sprintf(buf, "[%05u.%06u] ", ns, sec);
#endif
}
#else
#define print_time(time, buf) 0
#endif

#ifdef CONFIG_PRINT_CALLER_ID
static size_t print_caller(message_caller_id_t *id, char *buf)
{
    if (id->cpu_id == INVALID_CPU)
    {
        return sprintf(buf, "[BSP.0.00]");
    }
    return sprintf(buf, "[%02u.%02u.%02u]", id->cpu_id, id->thread_id, id->process_id);
}
#else
#define print_caller(id, buf) 0
#endif

static void append_char(char **pp, char *e, char c)
{
    if (*pp < e)
    {
        *(*pp)++ = c;
    }
}

static ssize_t msg_add_ext_text(char *buf, size_t size, const char *text, size_t text_len, unsigned char endc)
{
    char *p = buf;
    char *e = buf + size;
    size_t i;

    for (i = 0; i < text_len; i++)
    {
        unsigned char c = text[i];

        if (c < ' ' || c >= 127 || c == '\\')
        {
            p += scnprintf(p, e - p, "\\x%02x", c);
        }
        else
        {
            append_char(&p, e, c);
        }
    }
    append_char(&p, e, endc);

    return p - buf;
}

static ssize_t msg_print_ext_body(char *buf, size_t size, char *text, size_t text_len)
{
    return msg_add_ext_text(buf, size, text, text_len, '\n');
}

INTERNAL status_t console_try_lock_device(console_t *con)
{
    if (!con->device_try_lock)
    {
        return KERRNO_SUCCESS;
    }
    return con->device_try_lock(con);
}

INTERNAL status_t console_lock_device(console_t *con)
{
    if (!con->device_lock)
    {
        return KERRNO_SUCCESS;
    }
    return con->device_lock(con);
}

INTERNAL status_t console_unlock_device(console_t *con)
{
    if (!con->device_unlock)
    {
        return KERRNO_SUCCESS;
    }
    return con->device_unlock(con);
}

static const char *const console_level_names[] =
    {
        [LVL_FUNC_ENTER] = "ENTER",
        [LVL_DEBUG] = "DEBUG",
        [LVL_INFO] = "INFO",
        [LVL_SCHED] = "SCHED",
        [LVL_NOTICE] = "NOTICE",
        [LVL_WARNING] = "WARNING",
        [LVL_FIX] = "FIX_FOUND",
        [LVL_ERROR] = "ERR",
        [LVL_CRITICAL] = "CRIT",
        [LVL_ALERT] = "ALERT",
        [LVL_EMERG] = "EMERG",
        [LVL_BUG] = "BUG",
};

static bool suppress_message_printing(console_t *con, con_write_level_t level)
{
    return level >= con->console_loglevel;
}

static size_t info_print_prefix(message_info_t *info, char *buf)
{
    size_t len = 0;

    len += print_time(&info->time, buf + len);

    len += print_caller(&info->caller_id, buf + len);

    len += sprintf(buf + len, " %s", console_level_names[info->level]);

    if (IS_ENABLED(CONFIG_PRINT_CALLER_ID) || IS_ENABLED(CONFIG_PRINT_TIME))
    {
        buf[len++] = ' ';
        buf[len] = '\0';
    }

    return len;
}

static size_t record_print_text(cons_record_t *record)
{
    size_t text_len = record->info->text_len;
    size_t buf_size = record->text_buf_size;

    if (text_len > buf_size)
    {
        text_len = buf_size;
    }

    char prefix[CONFIG_CONSOLE_PREFIX_MAX];
    size_t prefix_len = info_print_prefix(record->info, prefix);

    char *text = record->text_buf;
    bool truncated = false;
    size_t len = 0;
    while (true)
    {
        size_t line_len;
        char *next = memchr(text, '\n', text_len);
        if (next)
        {
            line_len = next - text;
        }
        else
        {
            if (truncated)
            {
                break;
            }
            line_len = text_len;
        }

        if (len + prefix_len + text_len + 1 + 1 > buf_size)
        {
            if (len + prefix_len + line_len + 1 + 1 > buf_size)
            {
                break;
            }

            text_len = buf_size - len - prefix_len - 1 - 1;
            truncated = true;
        }

        memmove(text + prefix_len, text, text_len);
        memcpy(text, prefix, prefix_len);

        len += prefix_len + line_len + 1;
        if (text_len == line_len)
        {
            text[prefix_len + line_len] = '\n';
            break;
        }

        text += prefix_len + line_len + 1;

        text_len -= line_len + 1;
    }

    if (buf_size > 0)
    {
        record->text_buf[len] = 0;
    }

    return len;
}

bool console_get_next_message(console_t *con, console_message_t *cons_msg, uint64_t seq, bool is_extended, bool may_suppress)
{
    console_buffers_t *bufs = cons_msg->console_bufs;
    const size_t scratchbuf_sz = sizeof(bufs->scratchbuf);
    const size_t outbuf_sz = sizeof(bufs->outbuf);
    char *scratchbuf = bufs->scratchbuf;
    char *outbuf = bufs->outbuf;
    message_info_t info;
    cons_record_t r;
    size_t len = 0;

    if (is_extended)
    {
        cons_ringbuf_rec_init_rd(&r, &info, scratchbuf, scratchbuf_sz);
    }
    else
    {
        cons_ringbuf_rec_init_rd(&r, &info, outbuf, outbuf_sz);
    }

    if (!cons_ringbuf_read_valid(prb, seq, &r))
    {
        return false;
    }

    cons_msg->seq = r.info->seq;
    cons_msg->dropped = r.info->seq - seq;
    cons_msg->cpu_id = r.info->caller_id.cpu_id;
    cons_msg->pid = r.info->caller_id.process_id;
    cons_msg->log_level = r.info->log_level;
    cons_msg->level = r.info->level;

    if (false && may_suppress && suppress_message_printing(con, r.info->level))
    {
        goto out;
    }

    if (is_extended)
    {
        log_debug(MODULE, "is_extended");
        len = info_print_prefix(r.info, outbuf);
        len += msg_print_ext_body(outbuf + len, outbuf_sz - len, r.text_buf, r.info->text_len);
    }
    else
    {
        len = record_print_text(&r);
    }
out:
    cons_msg->outbuf_len = len;
    return true;
}

void console_lock()
{
    // might_sleep();

    // while (panic_on_other_cpu())
    // {
    //      msleep(1000);
    // }

    sem_wait(&console_sem);
    mutex_lock(&console_mutex);
    console_locked = 1;
    console_may_schedule = 1;
}

int is_console_locked()
{
    return console_locked;
}

static void __console_unlock()
{
    console_locked = 0;
    mutex_unlock(&console_mutex);
    sem_post(&console_sem);
}

void console_unlock()
{
    // struct console_flush_type ft;

    // printk_get_console_flush_type(&ft);
    // if (ft.legacy_direct)
    // 	__console_flush_and_unlock();
    // else
    __console_unlock();
}

static bool console_emit_next_record(console_t *con, bool *handover)
{
    bool is_extended = con->flags & CON_EXTENDED;
    char *outbuf = &shared_console_bufs.outbuf[0];
    console_message_t console_msg = {
        .console_bufs = &shared_console_bufs,
    };

    *handover = false;

    if (!console_get_next_message(con, &console_msg, con->seq, is_extended, true))
    {
        return false;
    }

    con->dropped += console_msg.dropped;

    if (console_msg.outbuf_len == 0)
    {
        con->seq = console_msg.seq + 1;
        goto skip;
    }

    if (con->dropped && !is_extended)
    {
        FUNC_NOT_IMPLEMENTED();
        // console_prepend_dropped(&console_msg, con->dropped);
        con->dropped = 0;
    }
    if (!panic_in_progress())
    {
        con->write(con, &console_msg, outbuf, console_msg.outbuf_len);
        con->seq = console_msg.seq + 1;
    }
    else
    {
        console_lock_device(con);
        // LOCK();

        con->write(con, &console_msg, outbuf, console_msg.outbuf_len);

        con->seq = console_msg.seq + 1;

        console_unlock_device(con);
        // UNLOCK();
    }
skip:
    return true;
}

static bool console_flush_one_record(bool do_cond_resched, uint64_t *next_seq, bool *handover, bool *try_again)
{
    bool any_usable = false;
    *try_again = false;

    int count = 0;

    list_node_t *node = console_list.head;
    while (node)
    {
        console_t *con = to_console(node);
        cons_flags_t flags = con->flags;
        uint64_t printk_seq;
        bool progress;

        // if ((flags & CON_NBCON) && (ft.nbcon_atomic || ft.nbcon_offload))
        // {
        // continue;
        // }

        if (!console_is_usable(con, flags, !do_cond_resched))
        {
            continue;
        }
        any_usable = true;

        {
            progress = console_emit_next_record(con, handover);
        }

        if (*handover)
        {
            goto fail;
        }

        {
            printk_seq = con->seq;
        }

        if (printk_seq > *next_seq)
        {
            *next_seq = printk_seq;
        }

        if (!progress)
        {
            if (count >= 10)
            {
                node = node->next;
                count = 0;
                continue;
            }
            count++;
            continue;
        }

        *try_again = true;

        if (panic_on_other_cpu())
        {
            goto fail_srcu;
        }
        node = node->next;
    }

    return any_usable;

fail_srcu:
fail:
    *try_again = false;
    return false;
}

static bool console_flush_all(bool do_cond_resched, uint64_t *next_seq, bool *handover)
{
    bool try_again;
    bool ret = true;

    if (next_seq)
    {
        *next_seq = 0;
    }
    if (handover)
    {
        *handover = false;
    }

    do
    {
        ret = console_flush_one_record(do_cond_resched, next_seq, handover, &try_again);
    } while (try_again);

    return ret;
}

static uint64_t get_init_console_seq(console_t *newcon, bool bootcon_registered)
{
    console_t *con;
    bool handover;
    uint64_t init_seq;

    if (newcon->flags & (CON_PRINTBUFFER | CON_BOOT))
    {
        mutex_lock(&syslog_lock);
        init_seq = syslog_seq;
        mutex_unlock(&syslog_lock);
    }
    else
    {
        // init_seq = prb_next_seq(prb);

        if (bootcon_registered && !keep_bootcon)
        {
            console_lock();

            if (!console_flush_all(true, &init_seq, &handover))
            {
                if (handover)
                {
                    console_lock();
                }

                // init_seq = prb_next_seq(prb);

                list_node_t *curr = console_list.head;
                while (curr)
                {
                    curr = curr->next;
                    con = to_console(curr);

                    uint64_t seq;

                    if (!(con->flags & CON_BOOT) || !(con->flags & CON_ENABLED))
                    {
                        continue;
                    }

                    if (con->flags & CON_NBCON)
                    {
                        FUNC_NOT_IMPLEMENTED();
                        // seq = nbcon_seq_read(con);
                    }
                    else
                    {
                        seq = con->seq;
                    }

                    if (seq < init_seq)
                    {
                        init_seq = seq;
                    }
                }
            }

            console_unlock();
        }
    }

    return init_seq;
}

INTERNAL status_t console_call_setup(console_t *newcon, char *options)
{
    ENTER_FUNC("%p, %p", newcon, options);
    status_t ret;

    if (!newcon->setup)
    {
        return KERRNO_SUCCESS;
    }

    /* Synchronize with possible boot console. */
    console_lock();
    ret = newcon->setup(newcon, options);
    console_unlock();

    return ret;
}

INTERNAL status_t try_enable_preferred_console(console_t *newcon, bool user_specified)
{
    ENTER_FUNC("%p, %s", newcon, user_specified BOOL_TO_STRING);
    int i;
    status_t ret;

    console_cmdline_t *c = console_cmdline;
    for (i = 0; i < MAX_CMDLINECONSOLES; i++, c++)
    {
        // trace_debug(MODULE, "c = %p", c);
        // trace_debug(MODULE, "c->name = %s, c->devname = %s", c->name, c->devname);
        /* Console not yet initialized? */
        if (!c->name[0] && !c->devname[0])
        {
            continue;
        }
        if (c->user_specified != user_specified)
        {
            continue;
        }
        if (!newcon->match || newcon->match(newcon, c->name, c->index, c->options) != 0)
        {
            static_assert(sizeof(c->name) == sizeof(newcon->name));
            if (strcmp(c->name, newcon->name) != 0)
            {
                continue;
            }
            if (newcon->index >= 0 && newcon->index != c->index)
            {
                continue;
            }
            if (newcon->index < 0)
            {
                newcon->index = c->index;
            }
            trace_debug(MODULE, "c->index = %u", c->index);

            ret = console_call_setup(newcon, c->options);
            if (ret)
            {
                return ret;
            }
        }
        newcon->flags |= CON_ENABLED;
        if (i == preferred_console)
        {
            newcon->flags |= CON_CONSDEV;
        }
        return KERRNO_SUCCESS;
    }

    KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
}

INTERNAL status_t console_try_enable_default(console_t *newcon)
{
    if (newcon->index < 0)
    {
        newcon->index = 0;
    }

    status_t ret = console_call_setup(newcon, NULL);
    if (ret)
    {
        return ret;
    }

    newcon->flags |= CON_ENABLED;

    if (newcon->device)
    {
        newcon->flags |= CON_CONSDEV;
    }
    return KERRNO_SUCCESS;
}

INTERNAL void set_user_specified(console_cmdline_t *c, bool user_specified)
{
    if (!user_specified)
    {
        return;
    }

    c->user_specified = true;
    console_set_on_cmdline = 1;
}

INTERNAL status_t __add_preferred_console(const char *name, const short idx, const char *devname, bool user_specified)
{
    ENTER_FUNC("%s, %u, %s, %s", name, idx, devname, user_specified BOOL_TO_STRING);
    if (!name && !devname)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "");
    }

    if (name && idx < 0)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "");
    }

    int i = 0;
    console_cmdline_t *c = console_cmdline;
    for (i = 0; i < MAX_CMDLINECONSOLES; i++)
    {
        if (!c->name[0] && !c->devname[0])
        {
            break;
        }
        if ((name && !strcmp(c->name, name) && c->index == idx) || (devname && !strcmp(c->devname, devname)))
        {
            preferred_console = i;
            set_user_specified(c, user_specified);
            return KERRNO_SUCCESS;
        }
        c++;
    }

    if (i == MAX_CMDLINECONSOLES)
    {
        KERRNO_RETURN(KERRNO_BAD_INDEX, "");
    }

    trace_debug(MODULE, "found index = %u", i);

    preferred_console = i;

    if (name)
    {
        strscpy(c->name, name, NULL);
        trace_debug(MODULE, "setting name c->name = %s, name = %s", c->name, name);
    }
    if (devname)
    {
        strscpy(c->devname, devname, NULL);
        trace_debug(MODULE, "setting name c->devname = %s", c->devname);
    }
    set_user_specified(c, user_specified);

    c->index = idx;
    return KERRNO_SUCCESS;
}

status_t console_setup(const char *str)
{
    // ENTER_FUNC("%s", str);
    static_assert(sizeof(console_cmdline[0].devname) >= sizeof(console_cmdline[0].name) + 4);

    if (!is_console_cmdline_zeroed)
    {
        for (size_t i = 0; i < MAX_CMDLINECONSOLES; i++)
        {
            memset(&console_cmdline[i], 0, sizeof(console_cmdline_t));
            console_cmdline[i].index = -1;
        }
        is_console_cmdline_zeroed = true;
    }

    char buf[sizeof(console_cmdline[0].devname)];

    char *devname = NULL;
    char *ttyname = NULL;
    if (strchr(str, ':'))
    {
        devname = buf;
    }
    else
    {
        ttyname = buf;
    }

    if (ttyname && isdigit(str[0]))
    {
        scnprintf(buf, sizeof(buf), "ttyS%s", str);
    }
    else
    {
        strscpy(buf, str, NULL);
    }

    // trace_debug(MODULE, "ttyname = %s, buf = %s", ttyname, buf);
    char _s[sizeof(console_cmdline[0].devname)];
    memcpy(_s, buf, sizeof(buf));
    char *s = _s;
    for (; *s; s++)
    {
        if ((ttyname && isdigit(*s)) || *s == ',')
        {
            break;
        }
    }
    // trace_debug(MODULE, "ttyname = %s, buf = %s", ttyname, buf);

    int idx;
    if (devname)
    {
        idx = -1;
    }
    else
    {
        strtoul(s, 10, (uint32_t *)&idx);
    }

    return __add_preferred_console(ttyname, idx, devname, true);
}

CMDLINE_PARAM("console", console_setup, 1);

status_t keep_bootcon_setup(const char *str)
{
    keep_bootcon = 1;
    trace_debug(MODULE, "debug: skip boot console de-registration.");

    return KERRNO_SUCCESS;
}

CMDLINE_PARAM("keep_bootcon", keep_bootcon_setup, 1);

bool console_is_registered(const console_t *_con)
{
    bool ret = false;

    LOCK();
    list_node_t *node = console_list.head;
    while (node)
    {
        if (node == &THIS_NODE(_con))
        {
            ret = true;
            break;
        }
        node = node->next;
    }
    UNLOCK();
    return ret;
}

static bool printk_time = IS_ENABLED(CONFIG_PRINT_TIME);

static inline void console_caller_id(message_caller_id_t *out_caller_id)
{
    cpu_t *entry = THISCPU();
    if (entry)
    {
        out_caller_id->cpu_id = entry->logical_id;
        if (entry->current)
        {
            out_caller_id->thread_id = entry->current->tid;
        }
    }
}

static uint16_t console_sprint(char *text, uint16_t size, const char *fmt, va_list args)
{
    uint16_t text_len;

    text_len = vscnprintf(text, size, fmt, args);

    // trace_console(text, text_len);

    return text_len;
}

status_t _console_store_ctx(console_write_ctx_t *ctx, size_t *out_size, va_list args)
{
    bool was_locked = true;
    if (!IS_LOCKED())
    {
        was_locked = false;
        LOCK();
    }

    va_list args2;
    va_copy(args2, args);
    char prefix_buf[8];
    size_t reserve_size = vsnprintf(prefix_buf, sizeof(prefix_buf), ctx->message, args2) + 1;
    va_end(args2);

    if (reserve_size > CONSOLE_RINGBUF_RECORD_MAX)
    {
        reserve_size = CONSOLE_RINGBUF_RECORD_MAX;
    }

    cons_ringbuf_resv_entry_t resv_entry;
    cons_record_t record;
    size_t trunc_msg_len = 0;
    cons_ringbuf_rec_init_wr(&record, reserve_size);
    if (cons_ringbuf_reserve(prb, &resv_entry, &record))
    {
        truncate_msg(&reserve_size, &trunc_msg_len);

        cons_ringbuf_rec_init_wr(&record, reserve_size + trunc_msg_len);
        if (cons_ringbuf_reserve(prb, &resv_entry, &record))
        {
        }
    }

    size_t text_len = console_sprint(record.text_buf, reserve_size, ctx->message, args);
    if (trunc_msg_len)
    {
        memcpy(&record.text_buf[text_len], trunc_msg, trunc_msg_len);
    }
    record.info->text_len = text_len + trunc_msg_len;
    record.info->log_level = ctx->log_level;
    record.info->level = ctx->level;
    record.info->time = ctx->time;
    record.info->caller_id.cpu_id = ctx->cpu_id;
    record.info->caller_id.thread_id = ctx->thread_id;

    cons_ringbuf_final_commit(&resv_entry);
    uint64_t next_seq;
    bool handover;
    console_flush_all(false, &next_seq, &handover);

    if (out_size)
    {
        *out_size = text_len + trunc_msg_len;
    }
    if (!was_locked)
    {
        UNLOCK();
    }
    return KERRNO_SUCCESS;
}

status_t console_write_args_full(con_write_level_t write_level, trace_level_t log_level, const char *file, const char *func, uint32_t line, const char *message, va_list args)
{
    if (console_list.count == 0)
    {
        return KERRNO_SUCCESS;
    }
    int32_64_t ns = timer_now_ns();
    time_t sec = timer_now_sec();
    console_write_ctx_t ctx = {
        .level = write_level,
        .log_level = log_level,
#ifdef DEBUG
        .file = file,
        .function = func,
        .line = line,
#endif
        .message = message,
        .message_length = strlen(message),
        .time = {
            .tv_nsec = ns % 10000,
            .tv_sec = sec,
        },
        .seq = sequence_number,
    };
    sequence_number++;

    cpu_t *cpu = cpu_arch_get_current();
    if (cpu)
    {
        ctx.cpu_id = cpu->logical_id;
        if (cpu->current)
        {
            ctx.thread_id = cpu->current->tid;
            if (cpu->current_proc)
            {
                ctx.process_id = cpu->current_proc->pid;
            }
        }
    }
    else
    {
        ctx.cpu_id = INVALID_CPU;
    }

    status_t ret = _console_store_ctx(&ctx, NULL, args);
    return ret;
}

status_t console_write_full(con_write_level_t write_level, trace_level_t log_level, const char *file, const char *func, uint32_t line, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    status_t ret = console_write_args_full(write_level, log_level, file, func, line, message, args);
    va_end(args);
    return ret;
}

status_t console_write(con_write_level_t write_level, trace_level_t log_level, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    status_t ret = console_write_args_full(write_level, log_level, NULL, NULL, 0, message, args);
    va_end(args);
    return ret;
}

status_t console_write_arg(con_write_level_t write_level, trace_level_t log_level, const char *message, va_list args)
{
    status_t ret = console_write_args_full(write_level, log_level, NULL, NULL, 0, message, args);
    return ret;
}

status_t console_register(console_t *newcon)
{
    ENTER_FUNC("%p", newcon);
    status_t ret = 0;

    LOCK();
    list_node_t *node = console_list.head;
    while (node)
    {
        console_t *con = to_console(node);
        if (node == &THIS_NODE(newcon))
        {
            trace_warn(MODULE, "console '%s%d' already registered", con->name, con->index);
            goto console_unlock;
        }
        node = node->next;
    }

    if ((newcon->flags & CON_BOOT) && have_real_console)
    {
        trace_warn(MODULE, "Too late to register bootconsole %s%d", newcon->name, newcon->index);
        goto console_unlock;
    }

    if (preferred_console < 0)
    {
        if (list_is_empty(&console_list) || !console_first()->device || console_first()->flags & CON_BOOT)
        {
            console_try_enable_default(newcon);
        }
    }

    ret = try_enable_preferred_console(newcon, true);

    if (have_boot_console && ((newcon->flags & (CON_CONSDEV | CON_BOOT)) == CON_CONSDEV))
    {
        newcon->flags &= ~CON_PRINTBUFFER;
    }

    if (ret == KERRNO_ENTRY_NOT_FOUND)
    {
        ret = try_enable_preferred_console(newcon, false);
    }

    if (ret || newcon->flags & CON_BRL)
    {
        if (newcon->flags & CON_NBCON)
        {
            // nbcon_free(newcon);
        }
        goto console_unlock;
    }

    if (newcon->flags & CON_BOOT)
    {
        have_boot_console = true;
    }

    console_lock_device(newcon);

    ADD_THIS_TO_LIST(console_list, newcon);

    console_unlock_device(newcon);

    trace_debug(MODULE, "%s%sconsole [%s%d] enabled", (newcon->flags & CON_NBCON) ? "" : "legacy ", (newcon->flags & CON_BOOT) ? "boot" : "", newcon->name, newcon->index);

console_unlock:
    UNLOCK();
    return KERRNO_SUCCESS;
}

status_t __init console_early_init()
{
    list_initialize(&console_list);
    mutex_init(&console_mutex);
    sem_init(&console_sem, 1);
    return n_tty_init();
}

CONSTRUCTOR_INITCALL(console_early_init);

status_t __init console_init()
{
    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_CONSOLE);

    return KERRNO_SUCCESS;
}

POSTCORE_INITCALL(console_init);


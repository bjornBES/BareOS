/*
 * File: n_tty.c
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/tty.h"
#include "drivers/tty/tty_buffer.h"
#include "drivers/tty/tty_ldisct.h"

#include "sync/mutex.h"

#include "ringbuf/ringbuf.h"

#include "kerrno.h"

#include "memory.h"
#include "string.h"

#include "ctype.h"
#include "math.h"

#include <signals/signals_uapi.h>
#include <binary.h>

#define N_TTY_BUF_SIZE        4096

#define ECHO_OP_START         0xFF
#define ECHO_OP_MOVE_BACK_COL 0x80
#define ECHO_OP_SET_CANON_COL 0x81
#define ECHO_OP_ERASE_TAB     0x82

typedef struct
{
    uint8_t buf[N_TTY_BUF_SIZE];
    size_t head;   // write position
    size_t tail;   // read position
    bool has_line; // a complete line is ready (\n was received)
} tty_line_buf_t;

typedef struct n_tty_data
{
    size_t read_head;
    size_t commit_head;
    size_t canon_head;
    size_t echo_head;
    size_t echo_commit;
    size_t echo_mark;
    DECLARE_BITMAP(char_map, 256);

    unsigned long overrun_time;
    unsigned int num_overrun;

    /* non-atomic */
    bool no_room;

    uint8_t lnext : 1;
    uint8_t erasing : 1;
    uint8_t raw : 1;
    uint8_t real_raw : 1;
    uint8_t icanon : 1;
    uint8_t push : 1;

    ringbuf_t *read_buffer;
    ringbuf_t *echo_buffer;
    uint8_t read_buf[N_TTY_BUF_SIZE];
    DECLARE_BITMAP(read_flags, N_TTY_BUF_SIZE);
    uint8_t echo_buf[N_TTY_BUF_SIZE];

    size_t read_tail;
    size_t line_start;

    size_t lookahead_count;

    uint32_t column;
    uint32_t canon_column;
    size_t echo_tail;

    mutex_t atomic_read_lock;
    mutex_t output_lock;
} n_tty_data_t;

#define MASK(x) ((x) & (N_TTY_BUF_SIZE - 1))

#define MODULE  "n-tty"

static inline size_t read_cnt(n_tty_data_t *ldata)
{
    return ldata->read_buffer->write_ptr - ldata->read_buffer->read_ptr;
}

static inline uint8_t read_buf(n_tty_data_t *ldata, size_t i)
{
    return ldata->read_buf[MASK(i)];
}

static inline uint8_t *read_buf_addr(n_tty_data_t *ldata, size_t i)
{
    return &ldata->read_buf[MASK(i)];
}

static inline void put_tty_queue(uint8_t c, n_tty_data_t *ldata)
{
    ringbuf_write(ldata->read_buffer, &c);
}

static inline int is_utf8_continuation(uint8_t c)
{
    return (c & 0xC0) == 0x80;
}

static inline int is_continuation(uint8_t c, const tty_struct_t *tty)
{
    return I_IUTF8(tty) && is_utf8_continuation(c);
}

// --- line buffer ---

// push a byte into the line buffer
status_t n_tty_linebuf_push(n_tty_data_t *ldata, uint8_t byte)
{
    // ENTER_FUNC("%p, 0x%x", ldata, byte);
    return ringbuf_write(ldata->read_buffer, &byte);
}

// erase the last byte from the line buffer
status_t n_tty_linebuf_erase(tty_struct_t *tty, n_tty_data_t *ldata)
{
    ringbuf_erase(ldata->read_buffer);
    if (L_ECHOE(tty))
    {
        // return tty_echo_erase(tty);
        return KERRNO_SUCCESSES;
    }
    return KERRNO_SUCCESSES;
}

static void n_tty_receive_char(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c)
{
    // ENTER_FUNC("%p, %p, 0x%x", tty, ldata, c);
    if (tty->flow.stopped && !tty->flow.tco_stopped && I_IXON(tty) && I_IXANY(tty))
    {
        start_tty(tty);
        // process_echoes(tty);
    }
    if (L_ECHO(tty))
    {
        // finish_erasing(ldata);
        /* Record the column of first canon char. */
        if (ldata->canon_head == ldata->read_head)
        {
            // echo_set_canon_col(ldata);
        }
        // echo_char(c, tty);
        // commit_echoes(tty);
    }
    status_t ret = 0;
    if (c == '\377' && I_PARMRK(tty))
    {
        ret = n_tty_linebuf_push(ldata, c);
        KERRNO_CHECK_RETURN(ret);
    }
    ret = n_tty_linebuf_push(ldata, c);
    KERRNO_CHECK_RETURN(ret);
}

static void n_tty_receive_char_flagged(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c, uint8_t flag)
{
    switch (flag)
    {
        case TTY_BREAK :
            FUNC_NOT_IMPLEMENTED();
            // tty_n_receive_break(tty);
            break;
        case TTY_PARITY :
        case TTY_FRAME :
            FUNC_NOT_IMPLEMENTED();
            // tty_n_receive_parity_error(tty, c);
            break;
        case TTY_OVERRUN :
            FUNC_NOT_IMPLEMENTED();
            // tty_n_receive_overrun(tty);
            break;
        default :
            tty_err(tty, "unknown flag %u", flag);
            break;
    }
}

static void n_tty_receive_char_lnext(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c, uint8_t flag)
{
    ENTER_FUNC("%p, %p, 0x%x, 0x%x", tty, ldata, c, flag);
    ldata->lnext = 0;
    if (LIKELY(flag == TTY_NORMAL))
    {
        if (I_ISTRIP(tty))
        {
            c &= 0x7F;
        }
        if (I_IUCLC(tty) && L_IEXTEN(tty))
        {
            c = tolower(c);
        }
        n_tty_receive_char(tty, ldata, c);
    }
    else
    {
        n_tty_receive_char_flagged(tty, ldata, c, flag);
    }
}

static void n_tty_receive_signal_char(tty_struct_t *tty, int signal, uint8_t c)
{
    ENTER_FUNC("%p, 0x%x, 0x%x", tty, signal, c);
    FUNC_NOT_IMPLEMENTED();
    // isig(signal, tty);
    if (I_IXON(tty))
    {
        start_tty(tty);
    }
    if (L_ECHO(tty))
    {
        // echo_char(c, tty);
        // commit_echoes(tty);
    }
    else
    {
        // process_echoes(tty);
    }
}

static bool n_tty_is_char_flow_ctrl(tty_struct_t *tty, uint8_t c)
{
    return c == START_CHAR(tty) || c == STOP_CHAR(tty);
}

static bool n_tty_receive_char_flow_ctrl(tty_struct_t *tty, uint8_t c, bool lookahead_done)
{
    ENTER_FUNC("%p, 0x%x, %s", tty, c, lookahead_done BOOL_TO_STRING);
    if (!n_tty_is_char_flow_ctrl(tty, c))
    {
        return false;
    }

    if (lookahead_done)
    {
        return true;
    }

    if (c == START_CHAR(tty))
    {
        start_tty(tty);
        // process_echoes(tty);
        return true;
    }

    /* STOP_CHAR */
    // stop_tty(tty);
    return true;
}

static void n_tty_receive_handle_newline(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c)
{
    ENTER_FUNC("%p, %p, 0x%x", tty, ldata, c);
    SET_BIT(MASK(ldata->read_buffer->write_ptr), ldata->read_flags);
    put_tty_queue(c, ldata);
    ldata->canon_head = ldata->read_buffer->write_ptr;
    // kill_fasync(&tty->fasync, SIGIO, POLL_IN);
    // wake_up_interruptible_poll(&tty->read_wait, EPOLLIN | EPOLLRDNORM);
}

static bool n_tty_receive_char_canon(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c)
{
    ENTER_FUNC("%p, %p, 0x%x", tty, ldata, c);
    if (c == ERASE_CHAR(tty) || c == KILL_CHAR(tty) || (c == WERASE_CHAR(tty) && L_IEXTEN(tty)))
    {
        // eraser(c, tty);
        // commit_echoes(tty);

        return true;
    }

    if (c == LNEXT_CHAR(tty) && L_IEXTEN(tty))
    {
        ldata->lnext = 1;
        if (L_ECHO(tty))
        {
            // finish_erasing(ldata);
            if (L_ECHOCTL(tty))
            {
                // echo_char_raw('^', ldata);
                // echo_char_raw('\b', ldata);
                // commit_echoes(tty);
            }
        }

        return true;
    }

    if (c == REPRINT_CHAR(tty) && L_ECHO(tty) && L_IEXTEN(tty))
    {
        size_t tail = ldata->canon_head;

        // finish_erasing(ldata);
        // echo_char(c, tty);
        // echo_char_raw('\n', ldata);
        while (MASK(tail) != MASK(ldata->read_head))
        {
            // echo_char(read_buf(ldata, tail), tty);
            tail++;
        }
        // commit_echoes(tty);

        return true;
    }

    if (c == '\n')
    {
        if (L_ECHO(tty) || L_ECHONL(tty))
        {
            // echo_char_raw('\n', ldata);
            // commit_echoes(tty);
        }
        n_tty_receive_handle_newline(tty, ldata, c);

        return true;
    }

    if (c == EOF_CHAR(tty))
    {
        c = DISABLED_CHAR;
        n_tty_receive_handle_newline(tty, ldata, c);

        return true;
    }

    if ((c == EOL_CHAR(tty)) || (c == EOL2_CHAR(tty) && L_IEXTEN(tty)))
    {

        if (L_ECHO(tty))
        {
            /* Record the column of first canon char. */
            if (ldata->canon_head == ldata->read_head)
            {
                // echo_set_canon_col(ldata);
            }
            // echo_char(c, tty);
            // commit_echoes(tty);
        }
        /*
         * XXX does PARMRK doubling happen for
         * EOL_CHAR and EOL2_CHAR?
         */
        if (c == '\377' && I_PARMRK(tty))
        {
            put_tty_queue(c, ldata);
        }

        n_tty_receive_handle_newline(tty, ldata, c);

        return true;
    }

    return false;
}

static void n_tty_receive_char_special(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c, bool lookahead_done)
{
    ENTER_FUNC("%p, %p, 0x%x, %s", tty, ldata, c, lookahead_done BOOL_TO_STRING);
    if (I_IXON(tty) && n_tty_receive_char_flow_ctrl(tty, c, lookahead_done))
    {
        return;
    }

    if (L_ISIG(tty))
    {
        if (c == INTR_CHAR(tty))
        {
            n_tty_receive_signal_char(tty, SIGINT, c);
            return;
        }
        else if (c == QUIT_CHAR(tty))
        {
            n_tty_receive_signal_char(tty, SIGQUIT, c);
            return;
        }
        else if (c == SUSP_CHAR(tty))
        {
            n_tty_receive_signal_char(tty, SIGTSTP, c);
            return;
        }
    }

    if (tty->flow.stopped && !tty->flow.tco_stopped && I_IXON(tty) && I_IXANY(tty))
    {
        start_tty(tty);
        // process_echoes(tty);
    }

    if (c == '\r')
    {
        if (I_IGNCR(tty))
        {
            return;
        }
        if (I_ICRNL(tty))
        {
            c = '\n';
        }
    }
    else if (c == '\n' && I_INLCR(tty))
    {
        c = '\r';
    }

    if (ldata->icanon && n_tty_receive_char_canon(tty, ldata, c))
    {
        return;
    }

    if (L_ECHO(tty))
    {
        // finish_erasing(ldata);
        if (c == '\n')
        {
            // echo_char_raw('\n', ldata);
        }
        else
        {
            /* Record the column of first canon char. */
            if (ldata->canon_head == ldata->read_head)
            {
                // echo_set_canon_col(ldata);
            }
            // echo_char(c, tty);
        }
        // commit_echoes(tty);
    }

    /* PARMRK doubling check */
    if (c == '\377' && I_PARMRK(tty))
    {
        put_tty_queue(c, ldata);
    }

    put_tty_queue(c, ldata);
}

static void n_tty_receive_buf_standard(tty_struct_t *tty, n_tty_data_t *ldata, const uint8_t *cp, const uint8_t *fp, size_t count, bool lookahead_done)
{
    ENTER_FUNC("%p, %p, %p, %p, 0x%x, %s", tty, ldata, cp, fp, count, lookahead_done BOOL_TO_STRING);
    uint8_t flag = TTY_NORMAL;
    while (count--)
    {
        uint8_t c = *cp++;

        if (fp)
        {
            flag = *fp++;
        }

        if (ldata->lnext)
        {
            n_tty_receive_char_lnext(tty, ldata, c, flag);
            continue;
        }

        if (UNLIKELY(flag != TTY_NORMAL))
        {
            n_tty_receive_char_flagged(tty, ldata, c, flag);
            continue;
        }

        if (I_ISTRIP(tty))
        {
            c &= 0x7F;
        }
        if (I_IUCLC(tty) && L_IEXTEN(tty))
        {
            c = tolower(c);
        }
        if (L_EXTPROC(tty))
        {
            n_tty_linebuf_push(ldata, c);
            continue;
        }

        if (TEST_BIT(c, ldata->char_map))
        {
            n_tty_receive_char_special(tty, ldata, c, lookahead_done);
        }
        else
        {
            n_tty_receive_char(tty, ldata, c);
        }
    }
}

static int do_output_char(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c, int space, int *out_status)
{
    if (!space)
    {
        *out_status = -1;
        return KERRNO_UNSUCCESS;
    }

    ssize_t bytes_written = 0;
    status_t ret = 0;
    switch (c)
    {
        case '\n' :
            if (O_ONLRET(tty))
            {
                ldata->column = 0;
            }
            if (O_ONLCR(tty))
            {
                if (space < 2)
                {
                    *out_status = -1;
                    return KERRNO_UNSUCCESS;
                }
                ldata->canon_column = ldata->column = 0;
                ret = tty->ops->write(tty, (const uint8_t *)"\r\n", 2, &bytes_written);
                *out_status = bytes_written;
                return ret;
            }
            ldata->canon_column = ldata->column;
            break;
        case '\r' :
            if (O_ONOCR(tty) && ldata->column == 0)
            {
                *out_status = 0;
                return KERRNO_SUCCESSES;
            }
            if (O_OCRNL(tty))
            {
                c = '\n';
                if (O_ONLRET(tty))
                {
                    ldata->canon_column = ldata->column = 0;
                }
                break;
            }
            ldata->canon_column = ldata->column = 0;
            break;
        case '\t' :
            int spaces = 8 - (ldata->column & 7);
            if (O_TABDLY(tty) == XTABS)
            {
                if (space < spaces)
                {
                    *out_status = -1;
                    return KERRNO_UNSUCCESS;
                }
                ldata->column += spaces;
                ret = tty->ops->write(tty, (const uint8_t *)"        ", spaces, &bytes_written);
                *out_status = bytes_written;
                return ret;
            }
            ldata->column += spaces;
            break;
        case '\b' :
            if (ldata->column > 0)
            {
                ldata->column--;
            }
            break;
        default :
            if (!iscntrl(c))
            {
                if (O_OLCUC(tty))
                {
                    c = toupper(c);
                }
                if (!is_continuation(c, tty))
                {
                    ldata->column++;
                }
            }
            break;
    }

    tty_put_char(tty, c);
    *out_status = 1;
    return KERRNO_SUCCESSES;
}

static status_t process_output(tty_struct_t *tty, n_tty_data_t *ldata, uint8_t c)
{
    mutex_lock(&ldata->output_lock);
    // guard(mutex)(&ldata->output_lock);
    
    size_t size = 0;
    status_t ret = tty_write_room(tty, &size);
    if (ret)
    {
        mutex_unlock(&ldata->output_lock);
        return ret;
    }
    int state = 0;
    ret = do_output_char(tty, ldata, c, size, &state);
    if (ret || state < 0)
    {
        mutex_unlock(&ldata->output_lock);
        return KERRNO_UNSUCCESS;
    }
    
    mutex_unlock(&ldata->output_lock);
    return KERRNO_SUCCESSES;
}

static ssize_t process_output_block(tty_struct_t *tty, n_tty_data_t *ldata, const uint8_t *buf, size_t count)
{
    mutex_lock(&ldata->output_lock);
    // guard(mutex)(&ldata->output_lock);

    size_t space;
    tty_write_room(tty, &space);
    if (space == 0)
    {
        return 0;
    }

    if (count > space)
    {
        count = space;
    }

    uint32_t i;
    const uint8_t *cp = buf;
    for (i = 0; i < count; i++, cp++)
    {
        uint8_t c = *cp;

        switch (c)
        {
            case '\n' :
                if (O_ONLRET(tty))
                {
                    ldata->column = 0;
                }
                if (O_ONLCR(tty))
                {
                    break;
                }
                ldata->canon_column = ldata->column;
                break;
            case '\r' :
                if (O_ONOCR(tty) && ldata->column == 0)
                {
                    break;
                }
                if (O_OCRNL(tty))
                {
                    break;
                }
                ldata->canon_column = ldata->column = 0;
                break;
            case '\t' :
                break;
            case '\b' :
                if (ldata->column > 0)
                {
                    ldata->column--;
                }
                break;
            default :
                if (!iscntrl(c))
                {
                    if (O_OLCUC(tty))
                    {
                        goto do_write;
                    }
                    if (!is_continuation(c, tty))
                    {
                        ldata->column++;
                    }
                }
                break;
        }
    }
do_write:
    ssize_t size;
    tty->ops->write(tty, buf, i, &size);
    mutex_unlock(&ldata->output_lock);
    LOG_RETURN(size);
    return size;
}

INTERNAL status_t n_tty_do_set_termios(tty_struct_t *tty, n_tty_data_t *ldata, const termios_t *old)
{
    if (!old || (old->c_lflag ^ tty->termios.c_lflag) & (ICANON | EXTPROC))
    {
        BITMAP_ZERO(ldata->read_flags, N_TTY_BUF_SIZE);
        ldata->line_start = ldata->read_tail;
        if (!L_ICANON(tty) || !read_cnt(ldata))
        {
            ldata->canon_head = ldata->read_tail;
            ldata->push = 0;
        }
        else
        {
            SET_BIT(MASK(ldata->read_head - 1), ldata->read_flags);
            ldata->canon_head = ldata->read_head;
            ldata->push = 1;
        }
        ldata->commit_head = ldata->read_head;
        ldata->erasing = 0;
        ldata->lnext = 0;
    }

    ldata->icanon = (L_ICANON(tty) != 0);

    if (I_ISTRIP(tty) || I_IUCLC(tty) || I_IGNCR(tty) || I_ICRNL(tty) || I_INLCR(tty) || L_ICANON(tty) || I_IXON(tty) || L_ISIG(tty) || L_ECHO(tty) || I_PARMRK(tty))
    {
        BITMAP_ZERO(ldata->char_map, 256);

        if (I_IGNCR(tty) || I_ICRNL(tty))
        {
            SET_BIT('\r', ldata->char_map);
        }
        if (I_INLCR(tty))
        {
            SET_BIT('\n', ldata->char_map);
        }

        if (L_ICANON(tty))
        {
            SET_BIT(ERASE_CHAR(tty), ldata->char_map);
            SET_BIT(KILL_CHAR(tty), ldata->char_map);
            SET_BIT(EOF_CHAR(tty), ldata->char_map);
            SET_BIT('\n', ldata->char_map);
            SET_BIT(EOL_CHAR(tty), ldata->char_map);
            if (L_IEXTEN(tty))
            {
                SET_BIT(WERASE_CHAR(tty), ldata->char_map);
                SET_BIT(LNEXT_CHAR(tty), ldata->char_map);
                SET_BIT(EOL2_CHAR(tty), ldata->char_map);
                if (L_ECHO(tty))
                {
                    SET_BIT(REPRINT_CHAR(tty), ldata->char_map);
                }
            }
        }
        if (I_IXON(tty))
        {
            SET_BIT(START_CHAR(tty), ldata->char_map);
            SET_BIT(STOP_CHAR(tty), ldata->char_map);
        }
        if (L_ISIG(tty))
        {
            SET_BIT(INTR_CHAR(tty), ldata->char_map);
            SET_BIT(QUIT_CHAR(tty), ldata->char_map);
            SET_BIT(SUSP_CHAR(tty), ldata->char_map);
        }
        CLEAR_BIT(DISABLED_CHAR, ldata->char_map);
        ldata->raw = 0;
        ldata->real_raw = 0;
    }
    else
    {
        ldata->raw = 1;
        if ((I_IGNBRK(tty) || (!I_BRKINT(tty) && !I_PARMRK(tty))) &&
            (I_IGNPAR(tty) || !I_INPCK(tty)) &&
            (tty->driver->flags & TTY_DRIVER_REAL_RAW))
        {
            ldata->real_raw = 1;
        }
        else
        {
            ldata->real_raw = 0;
        }
    }

    if (!I_IXON(tty) && old && (old->c_iflag & IXON) && !tty->flow.tco_stopped)
    {
        start_tty(tty);
        // process_echoes(tty);
    }

    // wake_up_interruptible(&tty->write_wait);
    // wake_up_interruptible(&tty->read_wait);
    return KERRNO_SUCCESSES;
}

static bool copy_from_read_buf(const tty_struct_t *tty, n_tty_data_t *ldata, uint8_t **kbp, size_t *nr)
{
    size_t head = ldata->commit_head; // smp_load_acquire(&ldata->commit_head);
    size_t tail = MASK(ldata->read_buffer->read_ptr);
    size_t n = min3(head - ldata->read_buffer->read_ptr, N_TTY_BUF_SIZE - tail, *nr);
    if (!n)
    {
        LOG_RETURN(false);
    }

    uint8_t *buffer = *kbp;
    size_t count = n;
    uint8_t from;
    status_t ret = 0;
    while (count)
    {
        size_t ring_size = 0;
        ret = ringbuf_read(ldata->read_buffer, &from, &ring_size);
        KERRNO_CHECK_RETURN(ret);
        if (ret)
        {
            if (ret == ENOBUFS)
            {
                break;
            }
        }
        *buffer = from;
        count--;
    }
    bool is_eof = false;
    if (n == 1 && from == EOF_CHAR(tty))
    {
        is_eof = true;
    }

    // tty_audit_add_data(tty, from, n);
    // zero_buffer(tty, from, n);
    // smp_store_release(&ldata->read_tail, ldata->read_tail + n);

    if (L_EXTPROC(tty) && ldata->icanon && is_eof && head == ldata->read_buffer->read_ptr)
    {
        LOG_RETURN(false);
    }

    *kbp += count;
    *nr -= count;

    LOG_RETURN(head != ldata->read_buffer->read_ptr);
}

static bool canon_copy_from_read_buf(const tty_struct_t *tty, n_tty_data_t *ldata, uint8_t **kbp, size_t *nr)
{

    if (!*nr)
    {
        LOG_RETURN(false);
    }

    size_t canon_head = ldata->canon_head; // smp_load_acquire(&ldata->canon_head);
    size_t n = min(*nr, canon_head - ldata->read_buffer->read_ptr);

    size_t tail = MASK(ldata->read_buffer->read_ptr);
    size_t size = (size_t)min(tail + n, N_TTY_BUF_SIZE);

    size_t eol = find_next_bit(ldata->read_flags, size, tail);
    size_t more = n - (size - tail);
    int found = 0;
    if (eol == N_TTY_BUF_SIZE && more)
    {
        /* scan wrapped without finding set bit */
        eol = find_first_bit(ldata->read_flags, more);
        found = eol != more;
    }
    else
    {
        found = eol != size;
    }

    n = eol - tail;
    if (n > N_TTY_BUF_SIZE)
    {
        n += N_TTY_BUF_SIZE;
    }
    size_t c = n + found;

    uint8_t c_eol;
    status_t ret = ringbuf_read_buffer(ldata->read_buffer, &c_eol, NULL);
    KERRNO_CHECK_RETURN(ret);
    if (!found || c_eol != DISABLED_CHAR)
    {
        n = c;
    }

    uint8_t *buffer = *kbp;
    size_t count = n;
    uint8_t from;
    while (count)
    {
        size_t ring_size = 0;
        ret = ringbuf_read(ldata->read_buffer, &from, &ring_size);
        KERRNO_CHECK_RETURN(ret);
        // trace_debug(MODULE, "from = 0x%x", from);
        if (ret)
        {
            if (ret == ENOBUFS)
            {
                break;
            }
        }
        *buffer = from;
        buffer++;
        count--;
    }
    count = n - count;
    // tty_copy(tty, *kbp, tail, n);
    *kbp += count;
    *nr -= count;

    if (found)
    {
        CLEAR_BIT(eol, ldata->read_flags);
    }

    if (found)
    {
        if (!ldata->push)
        {
            ldata->line_start = ldata->read_buffer->read_ptr;
        }
        else
        {
            ldata->push = 0;
        }
        // tty_audit_push();
        LOG_RETURN(false);
    }

    LOG_RETURN(ldata->read_buffer->read_ptr != canon_head);
}

static inline int input_available_p(const tty_struct_t *tty, n_tty_data_t *ldata, int poll)
{
    int amt = poll && !TIME_CHAR(tty) && MIN_CHAR(tty) ? MIN_CHAR(tty) : 1;

    if (ldata->icanon && !L_EXTPROC(tty))
    {
        return ldata->canon_head != ldata->read_buffer->read_ptr;
    }
    else
    {
        return ldata->commit_head - ldata->read_buffer->read_ptr >= amt;
    }
}

static status_t n_tty_wait_for_input(tty_struct_t *tty, n_tty_data_t *ldata, vfs_node_t *file, void *wait, long *timeout)
{
    ENTER_FUNC("%p, %p, %p, %p, %p", tty, ldata, file, wait, timeout);
    // if (TEST_BIT(TTY_OTHER_CLOSED, &tty->flags))
    // {
    //     return -EIO;
    // }
    // if (tty_hung_up_p(file))
    // {
    //     return 0;
    // }
    // if (TEST_BIT(TTY_HUPPING, &tty->flags))
    // {
    //     return 0;
    // }
    // if (!*timeout)
    // {
    //     return 0;
    // }
    // if (tty_io_nonblock(tty, file))
    // {
    //     return -EAGAIN;
    // }
    // if (signal_pending(current))
    // {
    //     return -ERESTARTSYS;
    // }

    while (ldata->canon_head == ldata->read_buffer->read_ptr)
    {
    }

    // up_read(&tty->termios_rwsem);
    // *timeout = wait_woken(wait, TASK_INTERRUPTIBLE, *timeout);
    // down_read(&tty->termios_rwsem);

    LOG_RETURN(KERRNO_SUCCESSES)
}

status_t n_tty_open(tty_struct_t *tty)
{
    n_tty_data_t *ldata;

    ldata = kzmalloc(sizeof(n_tty_data_t));
    if (!ldata)
    {
        KERRNO_RETURN(ENOMEM, "");
    }

    mutex_init(&ldata->atomic_read_lock);
    mutex_init(&ldata->output_lock);

    ringbuf_init(sizeof(uint8_t), N_TTY_BUF_SIZE, &ldata->read_buffer);

    tty->disc_data = ldata;
    // tty->closing = 0;

    BIT_UNSET(tty->flags, TTY_LDISC_HALTED);
    return n_tty_do_set_termios(tty, ldata, NULL);
}

status_t n_tty_receive(tty_struct_t *tty, wchar_t ch)
{
    ENTER_FUNC("%p, 0x%x", tty, ch);
    n_tty_data_t *ldata = tty->disc_data;
    size_t size = ch <= 0xFF;
    if (ch > 0xFF && ch <= 0xFFFF)
    {
        size = 2;
    }
    else if (ch > 0xFFFF && ch <= 0xFFFFFF)
    {
        size = 3;
    }
    if (ch > 0xFFFFFF && ch <= 0xFFFFFFFF)
    {
        size = 4;
    }
    n_tty_receive_buf_standard(tty, ldata, (void *)&ch, NULL, size, false);
    ldata->commit_head = ldata->read_head;
    // n_tty_process_event(tty, ldata, ch);
    // return n_tty_process_byte(tty, ldata, ch);
    return KERRNO_SUCCESSES;
}

status_t n_tty_set_termios(tty_struct_t *tty, const termios_t *old)
{
    n_tty_data_t *ldata = tty->disc_data;
    return n_tty_do_set_termios(tty, ldata, old);
}

status_t n_tty_read(tty_struct_t *tty, vfs_node_t *file, uint8_t *kbuf, size_t count, ssize_t *out_size)
{
    ENTER_FUNC("%p, %p, %p, %u, %p", tty, file, kbuf, count, out_size);
    uint8_t *buf = kbuf;
    n_tty_data_t *ldata = tty->disc_data;

    int minimum = 0;
    int time = 0;
    long timeout = __LONG_MAX__;
    // timeout = MAX_SCHEDULE_TIMEOUT;
    if (!ldata->icanon)
    {
        minimum = MIN_CHAR(tty);
        if (minimum)
        {
            time = (100 / 10) * TIME_CHAR(tty);
        }
        else
        {
            // timeout = (100 / 10) * TIME_CHAR(tty);
            minimum = 1;
        }
    }

    status_t ret = KERRNO_SUCCESSES;
    while (count)
    {
        if (!input_available_p(tty, ldata, 0))
        {
            // up_read(&tty->termios_rwsem);
            // tty_buffer_flush_work(tty->port);
            // down_read(&tty->termios_rwsem);
            if (!input_available_p(tty, ldata, 0))
            {
                ret = n_tty_wait_for_input(tty, ldata, file, NULL, &timeout);
                // if (ret)
                // {
                //     retval = ret;
                //     break;
                // }
                continue;
            }
        }
        if (ldata->icanon && !L_EXTPROC(tty))
        {
            if (canon_copy_from_read_buf(tty, ldata, &buf, &count))
            {
                goto more_to_be_read;
            }
        }
        else
        {
            // if (packet && kb == buf)
            // {
            //     *kb++ = TIOCPKT_DATA;
            //     nr--;
            // }

            if (copy_from_read_buf(tty, ldata, &buf, &count) && buf - kbuf >= minimum)
            {
                goto more_to_be_read;
            }
        }

        if (buf - kbuf >= minimum)
        {
            break;
        }
        if (time)
        {
            timeout = time;
        }
    }

    if (buf - kbuf)
    {
        *out_size = buf - kbuf;
    }

    return ret;

more_to_be_read:

    *out_size = buf - kbuf;
    return ret;
}

status_t n_tty_write(tty_struct_t *tty, vfs_node_t *file, const uint8_t *buf, size_t count, ssize_t *out_size)
{
    n_tty_data_t *ldata = tty->disc_data;
    const uint8_t *b = buf;
    ssize_t num;
    status_t ret = 0;
    while (1)
    {
        if (O_OPOST(tty))
        {
            while (count > 0)
            {
                num = process_output_block(tty, ldata, b, count);
                // trace_debug(MODULE, "num = %u", num);
                if (num < 0)
                {
                    if (num == EAGAIN)
                    {
                        break;
                    }
                    ret = num;
                    break;
                }
                b += num;
                count -= num;
                // trace_debug(MODULE, "count = %u", count);
                if (count == 0)
                {
                    break;
                }
                if (process_output(tty, ldata, *b))
                {
                    break;
                }
                b++;
                count--;
            }
            if (ret)
            {
                break;
            }
            if (tty->ops->flush_chars)
            {
                tty->ops->flush_chars(tty);
            }
        }
        if (!count)
        {
            break;
        }
    }

    *out_size = b - buf;
    LOG_RETURN(ret);
    // return ret;
}

const tty_ldisc_ops_t n_tty_ops = {
    .num = N_TTY,
    .name = "n_tty",
    .open = n_tty_open,
    // .close = n_tty_close,
    // .flush_buffer = n_tty_flush_buffer,
    .receive = n_tty_receive,
    .read = n_tty_read,
    .write = n_tty_write,
    .set_termios = n_tty_set_termios,
    // .poll = n_tty_poll,
    // .receive_buf = n_tty_receive_buf,
    // .write_wakeup = n_tty_write_wakeup,
    // .lookahead_buf = n_tty_lookahead_flow_ctrl,
};

status_t n_tty_init()
{
    return tty_register_ldisc(&n_tty_ops);
}

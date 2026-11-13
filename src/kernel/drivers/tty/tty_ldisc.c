/*
 * File: tty_ldisc.c
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/ttyt.h"
#include "drivers/tty/tty_ldisct.h"

#include "kerrno.h"

#include "memory.h"

#define MODULE    "tty-ldisc"

#define LOCK_NAME tty_ldisc_lock
DEF_LOCK(LOCK_NAME);

static const tty_ldisc_ops_t *tty_ldiscs[NR_LDISCS];

status_t tty_register_ldisc(const tty_ldisc_ops_t *new_ldisc)
{
    if (new_ldisc->num < N_TTY || new_ldisc->num >= NR_LDISCS)
    {
        KERRNO_RETURN(EINVAL, "");
    }

    LOCK();
    tty_ldiscs[new_ldisc->num] = new_ldisc;
    UNLOCK();

    return 0;
}

static status_t tty_get_ldops(int disc, const tty_ldisc_ops_t **ldisc_out)
{
    LOCK();
    const tty_ldisc_ops_t *ldops = tty_ldiscs[disc];
    if (ldops)
    {
        *ldisc_out = ldops;
    }
    else
    {
        KERRNO_RETURN(EINVAL, "");
    }
    UNLOCK();
    *ldisc_out = ldops;
    return KERRNO_SUCCESSES;
}

static status_t tty_ldisc_get(tty_struct_t *tty, int disc, tty_ldisc_t **out_ldisc)
{
    tty_ldisc_t *ld;
    const tty_ldisc_ops_t *ldops;

    if (disc < N_TTY || disc >= NR_LDISCS)
    {
        KERRNO_RETURN(EINVAL, "");
    }

    status_t ret = tty_get_ldops(disc, &ldops);
    if (ret)
    {
        FUNC_NOT_IMPLEMENTED();
    }

    /*
     * There is no way to handle allocation failure of only 16 bytes.
     * Let's simplify error handling and save more memory.
     */
    ld = kzmalloc(sizeof(tty_ldisc_t));
    ld->ops = ldops;
    ld->tty = tty;

    *out_ldisc = ld;
    return KERRNO_SUCCESSES;
}

static status_t tty_ldisc_open(tty_struct_t *tty, tty_ldisc_t *ld)
{
    WARN_ON(TEST_AND_SET_BIT(TTY_LDISC_OPEN, &tty->flags));
    if (ld->ops->open)
    {
        status_t ret;
        ret = ld->ops->open(tty);
        if (ret)
        {
            CLEAR_BIT(TTY_LDISC_OPEN, &tty->flags);
        }
        return ret;
    }
    return KERRNO_SUCCESSES;
}

static void tty_ldisc_close(tty_struct_t *tty, tty_ldisc_t *ld)
{
    WARN_ON(!TEST_BIT(TTY_LDISC_OPEN, &tty->flags));
    CLEAR_BIT(TTY_LDISC_OPEN, &tty->flags);
    if (ld->ops->close)
    {
        ld->ops->close(tty);
    }
}

status_t tty_ldisc_setup(tty_struct_t *tty, tty_struct_t *o_tty)
{
    ENTER_FUNC("%p, %p", tty, o_tty);
    status_t ret = tty_ldisc_open(tty, tty->ldisc);

    if (ret)
    {
        return ret;
    }

    if (o_tty)
    {
        /*
         * Called without o_tty->ldisc_sem held, as o_tty has been
         * just allocated and no one has a reference to it.
         */
        ret = tty_ldisc_open(o_tty, o_tty->ldisc);
        if (ret)
        {
            tty_ldisc_close(tty, tty->ldisc);
            return ret;
        }
    }
    return KERRNO_SUCCESSES;
}

status_t tty_ldisc_init(tty_struct_t *tty)
{
    tty_ldisc_t *ld;
    status_t ret = tty_ldisc_get(tty, N_TTY, &ld);

    if (ret)
    {
        return ret;
    }
    tty->ldisc = ld;
    return KERRNO_SUCCESSES;
}

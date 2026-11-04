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

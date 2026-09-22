/*
 * File: irq.c
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "irq/irq.h"
#include "ivt/ivt.h"

#include "asm/vectors_arch.h"

#include "kerrno.h"
#include "panic.h"

#define MODULE "irq"

typedef struct irq_handler
{
    uint8_t state;
    irq_handler_func_t handler;
    void *ctx;
} irq_handler_t;

irq_handler_t irq_handlers[MAX_VECTOR - IRQ_BASE];

irq_controller_t *current;

int irq_handler(intr_frame_t *regs)
{
    gsi_t gsi = regs->interrupt - IRQ_BASE;

    if (irq_is_masked(gsi))
    {
        irq_eoi(gsi);
        return KERRNO_SUCCESSES;
    }

    if (FLAG_IS_SET(irq_handlers[gsi].state, HANDLER_IN_USE))
    {
        irq_handlers[gsi].handler(regs, irq_handlers[gsi].ctx);
    }
    else
    {
        log_err(MODULE, "Unhandled IRQ %d base was %u", gsi, regs->interrupt);
        return KERRNO_SUCCESSES;
    }

    irq_eoi(gsi);
    return KERRNO_SUCCESSES;
}

status_t irq_initialize(irq_controller_t *(*get_ops)())
{
    irq_controller_t *controller = get_ops();
    if (controller->probe() != KERRNO_SUCCESSES)
    {
        current = NULL;
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "%s failed probe", controller->name);
    }

    if (controller->initialize() != KERRNO_SUCCESSES)
    {
        current = NULL;
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "%s failed initialize", controller->name);
    }

    current = get_ops();

    return KERRNO_SUCCESSES;
}

status_t irq_register_handler(gsi_t gsi, irq_handler_func_t handler, void *ctx, irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target)
{
    interrupt_vector_t vector = gsi + IRQ_BASE;
    trace_info(MODULE, "Registering IRQ handler (%p) on gsi %u/vector %d", handler, gsi, vector);
    status_t status = ivt_set_handler(vector, irq_handler);
    if (status != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_PERMISSION_DENIED, "ivt entry already in use");
    }
    irq_handlers[gsi].handler = handler;
    irq_handlers[gsi].state = 0;
    FLAG_SET(irq_handlers[gsi].state, HANDLER_IN_USE);
    irq_handlers[gsi].ctx = ctx;

    return KERRNO_SUCCESSES;
}

status_t irq_unregister_handler(gsi_t gsi)
{

    return KERRNO_SUCCESSES;
}

void irq_mask(gsi_t gsi)
{
}

void irq_unmask(gsi_t gsi)
{
}

bool irq_is_masked(gsi_t gsi)
{
    return false;
}

void irq_eoi(gsi_t gsi)
{
}

gsi_t irq_pick_free_gsi(gsi_t allowed_mask)
{
    ENTER_FUNC("0x%x", allowed_mask);

    for (uint32_t irq = 0; irq < MAX_VECTOR - IRQ_BASE; irq++)
    {
        // skip if not in allowed set
        if (!(allowed_mask & (1u << irq)))
        {
            continue;
        }

        // skip legacy IRQs that are already taken
        if (FLAG_IS_SET(irq_handlers[irq].state, HANDLER_IN_USE))
        {
            continue;
        }

        trace_debug(MODULE, "irq%u is free to take", irq);
        return irq;
    }

    // no free IRQ found — should not happen on sane hardware
    KERNEL_PANIC(MODULE, "pick_free_irq: no free IRQ in allowed set 0x%X", allowed_mask);
    return 0xFF;
}

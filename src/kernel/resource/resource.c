/*
 * File: resource.c
 * File Created: 27 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "resource/resource.h"
#include "resource/resourcet.h"

#include "debug/debug.h"

#include "assert/assert.h"

#include "kerrno.h"

#include "memory.h"

#include <defs.h>

#define MODULE "resource"

resource_space_t mem_space;
resource_space_t io_space;
resource_space_t irq_space;

status_t resource_space_init(resource_space_t *rs, res_space_t space, resource_size_t start, resource_size_t end)
{
    ENTER_FUNC("%p, %u, %p, %p", rs, space, start, end);
    rs->space = space;

    memset(&rs->root, 0, sizeof(resource_t));

    rs->root.start = start;
    rs->root.end = end;
    return KERRNO_SUCCESSES;
}

resource_t *resource_create()
{
    return kmalloc(sizeof(resource_t));
}

INTERNAL bool resource_assert_no_overlap(resource_t *parent, resource_t *new)
{
    ENTER_FUNC("%p, %p", parent, new);

    resource_t *cur = parent;
    if (cur->name == NULL)
    {
        cur = cur->child;
    }
    while (cur)
    {
        if (cur != new)
        {
            if ((new->end <= cur->start) || (new->start >= cur->end))
            {
                cur = cur->child;
                continue;
            }
            ASSERT_RETURN((new->end = cur->start) || (new->start = cur->end), false,
                          "RESOURCE overlap: [%p-%p) vs [%p-%p)",
                          new->start, new->end,
                          cur->start, cur->end);
        }
        if (cur->sibling && new->start >= cur->sibling->start && new->end < cur->sibling->end)
        {
            cur = cur->sibling;
        }
/*         else if (cur->child && new->start >= cur->child->start && new->end < cur->child->end)
        {
            cur = cur->child;
        } */
        else
        {
            return true;
        }
    }
    return true;
}

// insert sorted by start
status_t resource_do_insert(resource_t *parent, resource_t *new)
{
    if (resource_assert_no_overlap(parent, new) == false)
    {
        return KERRNO_UNSUCCESS;
    }
    log_debug(MODULE, "resource_do_insert");

    // find insertion point — list is sorted by start
    resource_t *cur = parent;
    while (cur->child && cur->child->start < new->start)
    {
        cur = cur->child;
    }
    trace_debug(MODULE, "found child at %p", cur);

    while (cur->sibling && cur->sibling->start < new->start)
    {
        cur = cur->sibling;
    }
    trace_debug(MODULE, "found sibling at %p", cur);

    if (parent->parent == NULL && parent->child == NULL && parent->name == NULL)
    {
        parent->child = new;
        new->parent = parent;
        return KERRNO_SUCCESSES;
    }

    if (new->start > cur->start && new->start < cur->end)
    {
        trace_crit(MODULE, "insert as child");
        new->parent = cur;
        cur->child = new;
        return KERRNO_SUCCESSES;
    }

    // insert before head
    if (cur->start > new->start)
    {
        trace_crit(MODULE, "insert before head");
        new->parent = cur;
        cur->child = new;
    }
    // insert after cur
    else
    {
        new->parent = cur->parent;
        new->sibling = cur->sibling;
        cur->sibling = new;
    }
    return KERRNO_SUCCESSES;
}

status_t resource_request(resource_t *parent, resource_t *out, resource_size_t start, resource_size_t end, const char *name, res_type_t type, res_flags_t flags)
{
    ENTER_FUNC("%p, %p, %p, %p, %s, %u, 0x%x", parent, out, start, end, name, type, flags);

    out->start = start;
    out->end = end;
    out->name = name;
    out->type = type;
    out->flags = flags;
    out->child = NULL;
    out->parent = NULL;
    out->sibling = NULL;

    resource_do_insert(parent, out);
    resource_dump(parent);
    return KERRNO_SUCCESSES;
}

status_t resource_insert(resource_t *parent, resource_t *entry)
{
    resource_do_insert(parent, entry);
    return KERRNO_SUCCESSES;
}

status_t resource_allocate(resource_t *parent, resource_t *out, resource_size_t size, resource_size_t align, const char *name, res_type_t type, res_flags_t flags)
{

    return KERRNO_SUCCESSES;
}

status_t resource_release(resource_t *res)
{

    return KERRNO_SUCCESSES;
}

resource_t *resource_find(resource_t *root, resource_size_t addr)
{
    ENTER_FUNC("%p, %p", root, addr);
    resource_t *vma = root;
    while (vma)
    {
        trace_debug(MODULE, "%s [%p-%p]", vma->name, vma->start, vma->end);
        trace_debug(MODULE, "%p >= %p && %p < %p", vma->start, addr, addr, vma->end);
        if (addr >= vma->start && addr < vma->end)
        {
            if (vma->child != NULL)
            {
                return resource_find(vma->child, addr);
            }
            return vma;
        }
        vma = vma->sibling;
    }
    return NULL;
}

void resource_dump(const resource_t *root)
{
    ENTER_FUNC("%p", root);
    trace_debug(MODULE, "%s [%p-%p]", root->name, root->start, root->end);
    resource_t *child = root->child;
    while (child)
    {
        trace_debug(MODULE, "\t%s [%p-%p]", child->name, child->start, child->end);
        resource_t *child2 = child->child;
        while (child2)
        {
            trace_debug(MODULE, "\t\t%s [%p-%p]", child2->name, child2->start, child2->end);
            child2 = child2->sibling;
        }
        child = child->sibling;
    }
}

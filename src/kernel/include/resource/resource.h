/*
 * File: resource.h
 * File Created: 27 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "resourcet.h"
#include <types.h>

extern resource_space_t mem_space;
extern resource_space_t io_space;
extern resource_space_t irq_space;

/**
 * @brief Initialize a root node covering the full given range for one space.
 * @pre Called during setup, before any driver attempts a claim.
 */
status_t resource_space_init(resource_space_t *rs, res_space_t space, resource_size_t start, resource_size_t end);

resource_t *resource_create();

/**
 * @brief Claim an exact [start, end] range as a child of @p parent.
 * @pre parent must belong to the same space and fully contain [start, end].
 * @note Fails if the range overlaps an existing sibling under parent.
 */
status_t resource_request(resource_t *parent, resource_t *out, resource_size_t start, resource_size_t end, const char *name, res_type_t type, res_flags_t flags);

/**
 * @brief Claim an exact [start, end] range as a child of @p parent.
 * @pre parent must belong to the same space and fully contain [start, end].
 * @note Fails if the range overlaps an existing sibling under parent.
 */
status_t resource_insert(resource_t *parent, resource_t *out);

/**
 * @brief Find and claim a free range of @p size under @p parent, honoring
 *        @p align, without the caller having to know free gaps in advance.
 * @note This is the PCI BAR allocation path.
 */
status_t resource_allocate(resource_t *parent, resource_t *out, resource_size_t size, resource_size_t align, const char *name, res_type_t type, res_flags_t flags);

/**
 * @brief Release a previously-claimed resource, detaching it from its
 *        parent's child list. Does not free @p res itself (caller-owned).
 */
status_t resource_release(resource_t *res);

/**
 * @brief Look up the resource node whose range contains @p addr, within
 *        the subtree rooted at @p root. Returns the deepest (most specific)
 *        matching node.
 */
resource_t *resource_find(resource_t *root, resource_size_t addr);

/**
 * @brief Debug dump of a subtree, one line per node, indented by depth.
 */
void resource_dump(const resource_t *root);

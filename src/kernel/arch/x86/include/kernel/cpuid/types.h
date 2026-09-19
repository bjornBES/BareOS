/*
 * File: types.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "leaf_types.h"

/*
 * Types for centralized CPUID tables:
 *
 * For internal use by the CPUID parser.
 */

/**
 * struct leaf_parse_info - CPUID query parse info
 * @nr_entries:	Number of valid entries filled by the CPUID parser
 */
typedef struct leaf_parse_info
{
    unsigned int nr_entries;
} leaf_parse_info_t;

/**
 * __CPUID_LEAF() - Define a CPUID output and parse info entry
 * @_name:	Struct type name of the CPUID leaf/subleaf (e.g. 'leaf_0x7_0'). Such
 *		types are defined at <cpuid/leaf_types.h> and follow the leaf_0xM_N
 *		format, where 0xM is the leaf and N is the subleaf.
 * @_count:	Number of storage entries to allocate for this leaf/subleaf.
 *
 * For a given leaf/subleaf, define an array of CPUID storage entries and an associated
 * query info structure.
 *
 * Use an array of storage entries to accommodate CPUID leaves with multiple subleaves
 * having the same output format.  This is common for hierarchical enumeration; e.g.,
 * CPUID(0x4), CPUID(0x12), and CPUID(0x8000001d).
 */
#define __CPUID_LEAF(_name, _count) \
    struct _name _name[_count];     \
    leaf_parse_info_t _name##_info

/**
 * CPUID_LEAF() - Define a 'struct cpuid_leaves' storage entry
 * @_leaf:	Leaf number, in compile-time 0xN format
 * @_subleaf:	Subleaf number, in compile-time decimal format
 *
 * Convenience wrapper around __CPUID_LEAF().
 */
#define CPUID_LEAF(_leaf, _subleaf)            \
    __CPUID_LEAF(leaf_##_leaf##_##_subleaf, 1)

#define __cpuid_leaf_first_subleaf(_l) \
    LEAF_##_l##_##SUBLEAF_N_FIRST
#define __cpuid_leaf_last_subleaf(_l) \
    LEAF_##_l##_##SUBLEAF_N_LAST

#define __cpuid_leaf_subleaf_count_min(_l) 2
#define __cpuid_leaf_subleaf_count_max(_l)                               \
    (__cpuid_leaf_last_subleaf(_l) - __cpuid_leaf_first_subleaf(_l) + 1)

/**
 * CPUID_LEAF_N() - Define a 'struct cpuid_leaves' storage entry
 * @_leaf:	Leaf number, in compile-time 0xN format
 * @_count:	Number of storage entries to allocate for that leaf. It must not exceed
 *		the limits defined at <cpuid/leaf_types.h>.
 *
 * Convenience wrapper around __CPUID_LEAF().
 */
#define CPUID_LEAF_N(_leaf, _count)                                 \
    static_assert(_count >= __cpuid_leaf_subleaf_count_min(_leaf)); \
    static_assert(_count <= __cpuid_leaf_subleaf_count_max(_leaf)); \
    __CPUID_LEAF(leaf_##_leaf##_##n, _count)

/*
 * struct cpuid_leaves - Parsed CPUID data
 */
typedef struct cpuid_leaves
{
    /*Leaf, Subleaf number (or max number of subleaves) */
    CPUID_LEAF(0x0, 0);
    CPUID_LEAF(0x1, 0);
    CPUID_LEAF(0x7, 0);
    CPUID_LEAF(0x7, 1);
} cpuid_leaves_t;

/*
 * File: processor_flags.h
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <util/binary.h>
#include "kernel/x86.h"

/*
 * Basic CPU control in CR0
 */
#define X86_CR0_PE_BIT 0  /* Protection Enable */
#define X86_CR0_PE     BIT(X86_CR0_PE_BIT)
#define X86_CR0_MP_BIT 1  /* Monitor Coprocessor */
#define X86_CR0_MP     BIT(X86_CR0_MP_BIT)
#define X86_CR0_EM_BIT 2  /* Emulation */
#define X86_CR0_EM     BIT(X86_CR0_EM_BIT)
#define X86_CR0_TS_BIT 3  /* Task Switched */
#define X86_CR0_TS     BIT(X86_CR0_TS_BIT)
#define X86_CR0_ET_BIT 4  /* Extension Type */
#define X86_CR0_ET     BIT(X86_CR0_ET_BIT)
#define X86_CR0_NE_BIT 5  /* Numeric Error */
#define X86_CR0_NE     BIT(X86_CR0_NE_BIT)
#define X86_CR0_WP_BIT 16 /* Write Protect */
#define X86_CR0_WP     BIT(X86_CR0_WP_BIT)
#define X86_CR0_AM_BIT 18 /* Alignment Mask */
#define X86_CR0_AM     BIT(X86_CR0_AM_BIT)
#define X86_CR0_NW_BIT 29 /* Not Write-through */
#define X86_CR0_NW     BIT(X86_CR0_NW_BIT)
#define X86_CR0_CD_BIT 30 /* Cache Disable */
#define X86_CR0_CD     BIT(X86_CR0_CD_BIT)
#define X86_CR0_PG_BIT 31 /* Paging */
#define X86_CR0_PG     BIT(X86_CR0_PG_BIT)

/*
 * Paging options in CR3
 */
#define X86_CR3_PWT_BIT          3 /* Page Write Through */
#define X86_CR3_PWT              BIT(X86_CR3_PWT_BIT)
#define X86_CR3_PCD_BIT          4 /* Page Cache Disable */
#define X86_CR3_PCD              BIT(X86_CR3_PCD_BIT)

#define X86_CR3_PCID_BITS        12

#define X86_CR3_LAM_U57_BIT      61 /* Activate LAM for userspace, 62:57 bits masked */
#define X86_CR3_LAM_U57          BIT(X86_CR3_LAM_U57_BIT)
#define X86_CR3_LAM_U48_BIT      62 /* Activate LAM for userspace, 62:48 bits masked */
#define X86_CR3_LAM_U48          BIT(X86_CR3_LAM_U48_BIT)
#define X86_CR3_PCID_NOFLUSH_BIT 63 /* Preserve old PCID */
#define X86_CR3_PCID_NOFLUSH     BIT(X86_CR3_PCID_NOFLUSH_BIT)

/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME_BIT        0  /* enable vm86 extensions */
#define X86_CR4_VME            BIT(X86_CR4_VME_BIT)
#define X86_CR4_PVI_BIT        1  /* virtual interrupts flag enable */
#define X86_CR4_PVI            BIT(X86_CR4_PVI_BIT)
#define X86_CR4_TSD_BIT        2  /* disable time stamp at ipl 3 */
#define X86_CR4_TSD            BIT(X86_CR4_TSD_BIT)
#define X86_CR4_DE_BIT         3  /* enable debugging extensions */
#define X86_CR4_DE             BIT(X86_CR4_DE_BIT)
#define X86_CR4_PSE_BIT        4  /* enable page size extensions */
#define X86_CR4_PSE            BIT(X86_CR4_PSE_BIT)
#define X86_CR4_PAE_BIT        5  /* enable physical address extensions */
#define X86_CR4_PAE            BIT(X86_CR4_PAE_BIT)
#define X86_CR4_MCE_BIT        6  /* Machine check enable */
#define X86_CR4_MCE            BIT(X86_CR4_MCE_BIT)
#define X86_CR4_PGE_BIT        7  /* enable global pages */
#define X86_CR4_PGE            BIT(X86_CR4_PGE_BIT)
#define X86_CR4_PCE_BIT        8  /* enable performance counters at ipl 3 */
#define X86_CR4_PCE            BIT(X86_CR4_PCE_BIT)
#define X86_CR4_OSFXSR_BIT     9  /* enable fast FPU save and restore */
#define X86_CR4_OSFXSR         BIT(X86_CR4_OSFXSR_BIT)
#define X86_CR4_OSXMMEXCPT_BIT 10 /* enable unmasked SSE exceptions */
#define X86_CR4_OSXMMEXCPT     BIT(X86_CR4_OSXMMEXCPT_BIT)
#define X86_CR4_UMIP_BIT       11 /* enable UMIP support */
#define X86_CR4_UMIP           BIT(X86_CR4_UMIP_BIT)
#define X86_CR4_LA57_BIT       12 /* enable 5-level page tables */
#define X86_CR4_LA57           BIT(X86_CR4_LA57_BIT)
#define X86_CR4_VMXE_BIT       13 /* enable VMX virtualization */
#define X86_CR4_VMXE           BIT(X86_CR4_VMXE_BIT)
#define X86_CR4_SMXE_BIT       14 /* enable safer mode (TXT) */
#define X86_CR4_SMXE           BIT(X86_CR4_SMXE_BIT)
#define X86_CR4_FSGSBASE_BIT   16 /* enable RDWRFSGS support */
#define X86_CR4_FSGSBASE       BIT(X86_CR4_FSGSBASE_BIT)
#define X86_CR4_PCIDE_BIT      17 /* enable PCID support */
#define X86_CR4_PCIDE          BIT(X86_CR4_PCIDE_BIT)
#define X86_CR4_OSXSAVE_BIT    18 /* enable xsave and xrestore */
#define X86_CR4_OSXSAVE        BIT(X86_CR4_OSXSAVE_BIT)
#define X86_CR4_SMEP_BIT       20 /* enable SMEP support */
#define X86_CR4_SMEP           BIT(X86_CR4_SMEP_BIT)
#define X86_CR4_SMAP_BIT       21 /* enable SMAP support */
#define X86_CR4_SMAP           BIT(X86_CR4_SMAP_BIT)
#define X86_CR4_PKE_BIT        22 /* enable Protection Keys support */
#define X86_CR4_PKE            BIT(X86_CR4_PKE_BIT)
#define X86_CR4_CET_BIT        23 /* enable Control-flow Enforcement Technology */
#define X86_CR4_CET            BIT(X86_CR4_CET_BIT)
#define X86_CR4_LASS_BIT       27 /* enable Linear Address Space Separation support */
#define X86_CR4_LASS           BIT(X86_CR4_LASS_BIT)
#define X86_CR4_LAM_SUP_BIT    28 /* LAM for supervisor pointers */
#define X86_CR4_LAM_SUP        BIT(X86_CR4_LAM_SUP_BIT)
#define X86_CR4_FRED_BIT       32 /* enable FRED kernel entry */
#define X86_CR4_FRED           BIT(X86_CR4_FRED_BIT)

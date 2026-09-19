/*
 * File: leaf_types.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

// Regex + Find and Replace are one of the best combos
// find: struct (leaf_0x[0-9a-fA-Fn_]+)\n(\{[\w\d\s\/:;\(\)\-\.,#\[\]=@\+\*'\\>!|]+\});
// replace: typedef struct $1\n$2 PACKED $1_t;
// BjornBEs - 17-09-2026 17:13

/*
 * Leaf 0x0
 * Maximum standard leaf + CPU vendor string
 */

typedef struct leaf_0x0_0
{
    // eax
    uint32_t max_std_leaf : 32; // Highest standard CPUID leaf
    // ebx
    uint32_t cpu_vendorid_0 : 32; // CPU vendor ID string bytes 0 - 3
    // ecx
    uint32_t cpu_vendorid_2 : 32; // CPU vendor ID string bytes 8 - 11
    // edx
    uint32_t cpu_vendorid_1 : 32; // CPU vendor ID string bytes 4 - 7
} PACKED leaf_0x0_0_t;

/*
 * Leaf 0x1
 * CPU FMS (Family/Model/Stepping) + standard feature flags
 */

typedef struct leaf_0x1_0
{
    // eax
    uint32_t stepping : 4;       // Stepping ID
    uint32_t base_model : 4;     // Base CPU model ID
    uint32_t base_family_id : 4; // Base CPU family ID
    uint32_t cpu_type : 2;       // CPU type
    uint32_t : 2;                // Reserved
    uint32_t ext_model : 4;      // Extended CPU model ID
    uint32_t ext_family : 8;     // Extended CPU family ID
    uint32_t : 4;                // Reserved
    // ebx
    uint32_t brand_id : 8;      // Brand index
    uint32_t clflush_size : 8;  // CLFLUSH instruction cache line size
    uint32_t n_logical_cpu : 8; // Logical CPU count
    uint32_t local_apic_id : 8; // Initial local APIC physical ID
    // ecx
    uint32_t sse3 : 1;               // Streaming SIMD Extensions 3 (SSE3)
    uint32_t pclmulqdq : 1;          // PCLMULQDQ instruction support
    uint32_t dtes64 : 1;             // 64-bit DS save area
    uint32_t monitor : 1;            // MONITOR/MWAIT support
    uint32_t dscpl : 1;              // CPL Qualified Debug Store
    uint32_t vmx : 1;                // Virtual Machine Extensions
    uint32_t smx : 1;                // Safer Mode Extensions
    uint32_t est : 1;                // Enhanced Intel SpeedStep
    uint32_t tm2 : 1;                // Thermal Monitor 2
    uint32_t ssse3 : 1;              // Supplemental SSE3
    uint32_t cntxt_id : 1;           // L1 Context ID
    uint32_t sdbg : 1;               // Silicon Debug
    uint32_t fma : 1;                // FMA extensions using YMM state
    uint32_t cx16 : 1;               // CMPXCHG16B instruction support
    uint32_t xtpr_update : 1;        // xTPR Update Control
    uint32_t pdcm : 1;               // Perfmon and Debug Capability
    uint32_t : 1;                    // Reserved
    uint32_t pcid : 1;               // Process-context identifiers
    uint32_t dca : 1;                // Direct Cache Access
    uint32_t sse4_1 : 1;             // SSE4.1
    uint32_t sse4_2 : 1;             // SSE4.2
    uint32_t x2apic : 1;             // X2APIC support
    uint32_t movbe : 1;              // MOVBE instruction support
    uint32_t popcnt : 1;             // POPCNT instruction support
    uint32_t tsc_deadline_timer : 1; // APIC timer one-shot operation
    uint32_t aes : 1;                // AES instructions
    uint32_t xsave : 1;              // XSAVE (and related instructions) support
    uint32_t osxsave : 1;            // XSAVE (and related instructions) are enabled by OS
    uint32_t avx : 1;                // AVX instructions support
    uint32_t f16c : 1;               // Half-precision floating-point conversion support
    uint32_t rdrand : 1;             // RDRAND instruction support
    uint32_t guest_status : 1;       // System is running as guest; (para-)virtualized system
    // edx
    uint32_t fpu : 1;       // Floating-Point Unit on-chip (x87)
    uint32_t vme : 1;       // Virtual-8086 Mode Extensions
    uint32_t de : 1;        // Debugging Extensions
    uint32_t pse : 1;       // Page Size Extension
    uint32_t tsc : 1;       // Time Stamp Counter
    uint32_t msr : 1;       // Model-Specific Registers (RDMSR and WRMSR support)
    uint32_t pae : 1;       // Physical Address Extensions
    uint32_t mce : 1;       // Machine Check Exception
    uint32_t cx8 : 1;       // CMPXCHG8B instruction
    uint32_t apic : 1;      // APIC on-chip
    uint32_t : 1;           // Reserved
    uint32_t sep : 1;       // SYSENTER, SYSEXIT, and associated MSRs
    uint32_t mtrr : 1;      // Memory Type Range Registers
    uint32_t pge : 1;       // Page Global Extensions
    uint32_t mca : 1;       // Machine Check Architecture
    uint32_t cmov : 1;      // Conditional Move Instruction
    uint32_t pat : 1;       // Page Attribute Table
    uint32_t pse36 : 1;     // Page Size Extension (36-bit)
    uint32_t psn : 1;       // Processor Serial Number
    uint32_t clflush : 1;   // CLFLUSH instruction
    uint32_t : 1;           // Reserved
    uint32_t ds : 1;        // Debug Store
    uint32_t acpi : 1;      // Thermal monitor and clock control
    uint32_t mmx : 1;       // MMX instructions
    uint32_t fxsr : 1;      // FXSAVE and FXRSTOR instructions
    uint32_t sse : 1;       // SSE instructions
    uint32_t sse2 : 1;      // SSE2 instructions
    uint32_t selfsnoop : 1; // Self Snoop
    uint32_t htt : 1;       // Hyper-threading
    uint32_t tm : 1;        // Thermal Monitor
    uint32_t ia64 : 1;      // Legacy IA-64 (Itanium) support bit, now reserved
    uint32_t pbe : 1;       // Pending Break Enable
} PACKED leaf_0x1_0_t;

/*
 * Leaf 0x2
 * Intel cache and TLB information one-byte descriptors
 */

typedef struct leaf_0x2_0
{
    // eax
    uint32_t iteration_count : 8; // Number of times this leaf must be queried
    uint32_t desc1 : 8;           // Descriptor #1
    uint32_t desc2 : 8;           // Descriptor #2
    uint32_t desc3 : 7;           // Descriptor #3
    uint32_t eax_invalid : 1;     // Descriptors 1-3 are invalid if set
    // ebx
    uint32_t desc4 : 8;       // Descriptor #4
    uint32_t desc5 : 8;       // Descriptor #5
    uint32_t desc6 : 8;       // Descriptor #6
    uint32_t desc7 : 7;       // Descriptor #7
    uint32_t ebx_invalid : 1; // Descriptors 4-7 are invalid if set
    // ecx
    uint32_t desc8 : 8;       // Descriptor #8
    uint32_t desc9 : 8;       // Descriptor #9
    uint32_t desc10 : 8;      // Descriptor #10
    uint32_t desc11 : 7;      // Descriptor #11
    uint32_t ecx_invalid : 1; // Descriptors 8-11 are invalid if set
    // edx
    uint32_t desc12 : 8;      // Descriptor #12
    uint32_t desc13 : 8;      // Descriptor #13
    uint32_t desc14 : 8;      // Descriptor #14
    uint32_t desc15 : 7;      // Descriptor #15
    uint32_t edx_invalid : 1; // Descriptors 12-15 are invalid if set
} PACKED leaf_0x2_0_t;

/*
 * Leaf 0x4
 * Intel deterministic cache parameters
 */

typedef struct leaf_0x4_n
{
    // eax
    uint32_t cache_type : 5;           // Cache type field
    uint32_t cache_level : 3;          // Cache level (1-based)
    uint32_t cache_self_init : 1;      // Self-initializing cache level
    uint32_t fully_associative : 1;    // Fully-associative cache
    uint32_t : 4;                      // Reserved
    uint32_t num_threads_sharing : 12; // Number logical CPUs sharing this cache
    uint32_t num_cores_on_die : 6;     // Number of cores in the physical package
    // ebx
    uint32_t cache_linesize : 12;    // System coherency line size (0-based)
    uint32_t cache_npartitions : 10; // Physical line partitions (0-based)
    uint32_t cache_nways : 10;       // Ways of associativity (0-based)
    // ecx
    uint32_t cache_nsets : 31; // Cache number of sets (0-based)
    uint32_t : 1;              // Reserved
    // edx
    uint32_t wbinvd_rll_no_guarantee : 1; // WBINVD/INVD not guaranteed for Remote Lower-Level caches
    uint32_t ll_inclusive : 1;            // Cache is inclusive of Lower-Level caches
    uint32_t complex_indexing : 1;        // Not a direct-mapped cache (complex function)
    uint32_t : 29;                        // Reserved
} PACKED leaf_0x4_n_t;

#define LEAF_0x4_SUBLEAF_N_FIRST 0
#define LEAF_0x4_SUBLEAF_N_LAST  31

/*
 * Leaf 0x5
 * MONITOR/MWAIT instructions
 */

typedef struct leaf_0x5_0
{
    // eax
    uint32_t min_mon_size : 16; // Smallest monitor-line size, in bytes
    uint32_t : 16;              // Reserved
    // ebx
    uint32_t max_mon_size : 16; // Largest monitor-line size, in bytes
    uint32_t : 16;              // Reserved
    // ecx
    uint32_t mwait_ext : 1;       // MONITOR/MWAIT extensions
    uint32_t mwait_irq_break : 1; // Interrupts as a break event for MWAIT
    uint32_t : 30;                // Reserved
    // edx
    uint32_t n_c0_substates : 4; // Number of C0 sub C-states
    uint32_t n_c1_substates : 4; // Number of C1 sub C-states
    uint32_t n_c2_substates : 4; // Number of C2 sub C-states
    uint32_t n_c3_substates : 4; // Number of C3 sub C-states
    uint32_t n_c4_substates : 4; // Number of C4 sub C-states
    uint32_t n_c5_substates : 4; // Number of C5 sub C-states
    uint32_t n_c6_substates : 4; // Number of C6 sub C-states
    uint32_t n_c7_substates : 4; // Number of C7 sub C-states
} PACKED leaf_0x5_0_t;

/*
 * Leaf 0x6
 * Thermal and power management
 */

typedef struct leaf_0x6_0
{
    // eax
    uint32_t digital_temp : 1;          // Digital temperature sensor
    uint32_t turbo_boost : 1;           // Intel Turbo Boost
    uint32_t lapic_timer_always_on : 1; // Always-Running APIC Timer (not affected by p-state)
    uint32_t : 1;                       // Reserved
    uint32_t power_limit_event : 1;     // Power Limit Notification (PLN) event
    uint32_t ecmd : 1;                  // Clock modulation duty cycle extension
    uint32_t package_thermal : 1;       // Package thermal management
    uint32_t hwp_base_regs : 1;         // HWP (Hardware P-states) base registers
    uint32_t hwp_notify : 1;            // HWP notification (IA32_HWP_INTERRUPT MSR)
    uint32_t hwp_activity_window : 1;   // HWP activity window (IA32_HWP_REQUEST[bits 41:32])
    uint32_t hwp_energy_perf_pr : 1;    // HWP Energy Performance Preference
    uint32_t hwp_package_req : 1;       // HWP Package Level Request
    uint32_t : 1;                       // Reserved
    uint32_t hdc_base_regs : 1;         // HDC base registers
    uint32_t turbo_boost_3_0 : 1;       // Intel Turbo Boost Max 3.0
    uint32_t hwp_capabilities : 1;      // HWP Highest Performance change
    uint32_t hwp_peci_override : 1;     // HWP PECI override
    uint32_t hwp_flexible : 1;          // Flexible HWP
    uint32_t hwp_fast : 1;              // IA32_HWP_REQUEST MSR fast access mode
    uint32_t hw_feedback : 1;           // HW_FEEDBACK MSRs
    uint32_t hwp_ignore_idle : 1;       // Ignoring idle logical CPU HWP request is supported
    uint32_t : 1;                       // Reserved
    uint32_t hwp_ctl : 1;               // IA32_HWP_CTL MSR
    uint32_t thread_director : 1;       // Intel thread director
    uint32_t therm_interrupt_bit25 : 1; // IA32_THERM_INTERRUPT MSR bit 25
    uint32_t : 7;                       // Reserved
    // ebx
    uint32_t n_therm_thresholds : 4; // Digital thermometer thresholds
    uint32_t : 28;                   // Reserved
    // ecx
    uint32_t aperf_mperf : 1;          // MPERF/APERF MSRs (effective frequency interface)
    uint32_t : 2;                      // Reserved
    uint32_t energy_perf_bias : 1;     // IA32_ENERGY_PERF_BIAS MSR
    uint32_t : 4;                      // Reserved
    uint32_t hw_feedback_nclasses : 8; // Number of Intel Thread Director classes
    uint32_t : 16;                     // Reserved
    // edx
    uint32_t perfcap_reporting : 1;     // Performance capability reporting
    uint32_t encap_reporting : 1;       // Energy efficiency capability reporting
    uint32_t : 6;                       // Reserved
    uint32_t feedback_sz : 4;           // Feedback interface structure size, in 4K pages
    uint32_t : 4;                       // Reserved
    uint32_t this_lcpu_hwfdbk_idx : 16; // This logical CPU hardware feedback interface index
} PACKED leaf_0x6_0_t;

/*
 * Leaf 0x7
 * Extended CPU features
 */

typedef struct leaf_0x7_0
{
    // eax
    uint32_t leaf7_n_subleaves : 32; // Number of leaf 0x7 subleaves
    // ebx
    uint32_t fsgsbase : 1;        // FSBASE/GSBASE read/write
    uint32_t tsc_adjust : 1;      // IA32_TSC_ADJUST MSR
    uint32_t sgx : 1;             // Intel SGX (Software Guard Extensions)
    uint32_t bmi1 : 1;            // Bit manipulation extensions group 1
    uint32_t hle : 1;             // Hardware Lock Elision
    uint32_t avx2 : 1;            // AVX2 instruction set
    uint32_t fdp_excptn_only : 1; // FPU Data Pointer updated only on x87 exceptions
    uint32_t smep : 1;            // Supervisor Mode Execution Protection
    uint32_t bmi2 : 1;            // Bit manipulation extensions group 2
    uint32_t erms : 1;            // Enhanced REP MOVSB/STOSB
    uint32_t invpcid : 1;         // INVPCID instruction (Invalidate Processor Context ID)
    uint32_t rtm : 1;             // Intel restricted transactional memory
    uint32_t pqm : 1;             // Intel RDT-CMT / AMD Platform-QoS cache monitoring
    uint32_t zero_fcs_fds : 1;    // Deprecated FPU CS/DS (stored as zero)
    uint32_t mpx : 1;             // Intel memory protection extensions
    uint32_t rdt_a : 1;           // Intel RDT / AMD Platform-QoS Enforcement
    uint32_t avx512f : 1;         // AVX-512 foundation instructions
    uint32_t avx512dq : 1;        // AVX-512 double/quadword instructions
    uint32_t rdseed : 1;          // RDSEED instruction
    uint32_t adx : 1;             // ADCX/ADOX instructions
    uint32_t smap : 1;            // Supervisor mode access prevention
    uint32_t avx512ifma : 1;      // AVX-512 integer fused multiply add
    uint32_t : 1;                 // Reserved
    uint32_t clflushopt : 1;      // CLFLUSHOPT instruction
    uint32_t clwb : 1;            // CLWB instruction
    uint32_t intel_pt : 1;        // Intel processor trace
    uint32_t avx512pf : 1;        // AVX-512 prefetch instructions
    uint32_t avx512er : 1;        // AVX-512 exponent/reciprocal instructions
    uint32_t avx512cd : 1;        // AVX-512 conflict detection instructions
    uint32_t sha : 1;             // SHA/SHA256 instructions
    uint32_t avx512bw : 1;        // AVX-512 byte/word instructions
    uint32_t avx512vl : 1;        // AVX-512 VL (128/256 vector length) extensions
    // ecx
    uint32_t prefetchwt1 : 1;      // PREFETCHWT1 (Intel Xeon Phi only)
    uint32_t avx512vbmi : 1;       // AVX-512 Vector byte manipulation instructions
    uint32_t umip : 1;             // User mode instruction protection
    uint32_t pku : 1;              // Protection keys for user-space
    uint32_t ospke : 1;            // OS protection keys enable
    uint32_t waitpkg : 1;          // WAITPKG instructions
    uint32_t avx512_vbmi2 : 1;     // AVX-512 vector byte manipulation instructions group 2
    uint32_t cet_ss : 1;           // CET shadow stack features
    uint32_t gfni : 1;             // Galois field new instructions
    uint32_t vaes : 1;             // Vector AES instructions
    uint32_t vpclmulqdq : 1;       // VPCLMULQDQ 256-bit instruction
    uint32_t avx512_vnni : 1;      // Vector neural network instructions
    uint32_t avx512_bitalg : 1;    // AVX-512 bitwise algorithms
    uint32_t tme : 1;              // Intel total memory encryption
    uint32_t avx512_vpopcntdq : 1; // AVX-512: POPCNT for vectors of DWORD/QWORD
    uint32_t : 1;                  // Reserved
    uint32_t la57 : 1;             // 57-bit linear addresses (five-level paging)
    uint32_t mawau_val_lm : 5;     // BNDLDX/BNDSTX MAWAU value in 64-bit mode
    uint32_t rdpid : 1;            // RDPID instruction
    uint32_t key_locker : 1;       // Intel key locker
    uint32_t bus_lock_detect : 1;  // OS bus-lock detection
    uint32_t cldemote : 1;         // CLDEMOTE instruction
    uint32_t : 1;                  // Reserved
    uint32_t movdiri : 1;          // MOVDIRI instruction
    uint32_t movdir64b : 1;        // MOVDIR64B instruction
    uint32_t enqcmd : 1;           // Enqueue stores (ENQCMD{,S})
    uint32_t sgx_lc : 1;           // Intel SGX launch configuration
    uint32_t pks : 1;              // Protection keys for supervisor-mode pages
    // edx
    uint32_t : 1;                     // Reserved
    uint32_t sgx_keys : 1;            // Intel SGX attestation services
    uint32_t avx512_4vnniw : 1;       // AVX-512 neural network instructions
    uint32_t avx512_4fmaps : 1;       // AVX-512 multiply accumulation single precision
    uint32_t fsrm : 1;                // Fast short REP MOVSB
    uint32_t uintr : 1;               // User interrupts
    uint32_t : 2;                     // Reserved
    uint32_t avx512_vp2intersect : 1; // VP2INTERSECT{D,Q} instructions
    uint32_t srbds_ctrl : 1;          // SRBDS mitigation MSR
    uint32_t md_clear : 1;            // VERW MD_CLEAR microcode
    uint32_t rtm_always_abort : 1;    // XBEGIN (RTM transaction) always aborts
    uint32_t : 1;                     // Reserved
    uint32_t tsx_force_abort : 1;     // MSR TSX_FORCE_ABORT, RTM_ABORT bit
    uint32_t serialize : 1;           // SERIALIZE instruction
    uint32_t hybrid_cpu : 1;          // The CPU is identified as a 'hybrid part'
    uint32_t tsxldtrk : 1;            // TSX suspend/resume load address tracking
    uint32_t : 1;                     // Reserved
    uint32_t pconfig : 1;             // PCONFIG instruction
    uint32_t arch_lbr : 1;            // Intel architectural LBRs
    uint32_t cet_ibt : 1;             // CET indirect branch tracking
    uint32_t : 1;                     // Reserved
    uint32_t amx_bf16 : 1;            // AMX-BF16: tile bfloat16
    uint32_t avx512_fp16 : 1;         // AVX-512 FP16 instructions
    uint32_t amx_tile : 1;            // AMX-TILE: tile architecture
    uint32_t amx_int8 : 1;            // AMX-INT8: tile 8-bit integer
    uint32_t spec_ctrl : 1;           // Speculation Control (IBRS/IBPB: indirect branch restrictions)
    uint32_t intel_stibp : 1;         // Single thread indirect branch predictors
    uint32_t flush_l1d : 1;           // FLUSH L1D cache: IA32_FLUSH_CMD MSR
    uint32_t arch_capabilities : 1;   // Intel IA32_ARCH_CAPABILITIES MSR
    uint32_t core_capabilities : 1;   // IA32_CORE_CAPABILITIES MSR
    uint32_t spec_ctrl_ssbd : 1;      // Speculative store bypass disable
} PACKED leaf_0x7_0_t;

typedef struct leaf_0x7_1
{
    // eax
    uint32_t : 4;                  // Reserved
    uint32_t avx_vnni : 1;         // AVX-VNNI instructions
    uint32_t avx512_bf16 : 1;      // AVX-512 bfloat16 instructions
    uint32_t lass : 1;             // Linear address space separation
    uint32_t cmpccxadd : 1;        // CMPccXADD instructions
    uint32_t arch_perfmon_ext : 1; // ArchPerfmonExt: leaf 0x23
    uint32_t : 1;                  // Reserved
    uint32_t fzrm : 1;             // Fast zero-length REP MOVSB
    uint32_t fsrs : 1;             // Fast short REP STOSB
    uint32_t fsrc : 1;             // Fast Short REP CMPSB/SCASB
    uint32_t : 4;                  // Reserved
    uint32_t fred : 1;             // FRED: Flexible return and event delivery transitions
    uint32_t lkgs : 1;             // LKGS: Load 'kernel' (userspace) GS
    uint32_t wrmsrns : 1;          // WRMSRNS instruction (WRMSR-non-serializing)
    uint32_t nmi_src : 1;          // NMI-source reporting with FRED event data
    uint32_t amx_fp16 : 1;         // AMX-FP16: FP16 tile operations
    uint32_t hreset : 1;           // HRESET (Thread director history reset)
    uint32_t avx_ifma : 1;         // Integer fused multiply add
    uint32_t : 2;                  // Reserved
    uint32_t lam : 1;              // Linear address masking
    uint32_t rd_wr_msrlist : 1;    // RDMSRLIST/WRMSRLIST instructions
    uint32_t : 4;                  // Reserved
    // ebx
    uint32_t intel_ppin : 1; // Protected processor inventory number (PPIN{,_CTL} MSRs)
    uint32_t : 31;           // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 4;                // Reserved
    uint32_t avx_vnni_int8 : 1;  // AVX-VNNI-INT8 instructions
    uint32_t avx_ne_convert : 1; // AVX-NE-CONVERT instructions
    uint32_t : 2;                // Reserved
    uint32_t amx_complex : 1;    // AMX-COMPLEX instructions (starting from Granite Rapids)
    uint32_t : 5;                // Reserved
    uint32_t prefetchit_0_1 : 1; // PREFETCHIT0/1 instructions
    uint32_t : 3;                // Reserved
    uint32_t cet_sss : 1;        // CET supervisor shadow stacks safe to use
    uint32_t : 13;               // Reserved
} PACKED leaf_0x7_1_t;

typedef struct leaf_0x7_2
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t intel_psfd : 1;     // Intel predictive store forward disable
    uint32_t ipred_ctrl : 1;     // MSR bits IA32_SPEC_CTRL.IPRED_DIS_{U,S}
    uint32_t rrsba_ctrl : 1;     // MSR bits IA32_SPEC_CTRL.RRSBA_DIS_{U,S}
    uint32_t ddp_ctrl : 1;       // MSR bit IA32_SPEC_CTRL.DDPD_U
    uint32_t bhi_ctrl : 1;       // MSR bit IA32_SPEC_CTRL.BHI_DIS_S
    uint32_t mcdt_no : 1;        // MCDT mitigation not needed
    uint32_t uclock_disable : 1; // UC-lock disable
    uint32_t : 25;               // Reserved
} PACKED leaf_0x7_2_t;

/*
 * Leaf 0x9
 * Intel DCA (Direct Cache Access)
 */

typedef struct leaf_0x9_0
{
    // eax
    uint32_t dca_enabled_in_bios : 1; // DCA is enabled in BIOS
    uint32_t : 31;                    // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x9_0_t;

/*
 * Leaf 0xa
 * Intel PMU (Performance Monitoring Unit)
 */

typedef struct leaf_0xa_0
{
    // eax
    uint32_t pmu_version : 8;     // Performance monitoring unit version ID
    uint32_t num_counters_gp : 8; // Number of general-purpose PMU counters per logical CPU
    uint32_t bit_width_gp : 8;    // Bitwidth of PMU general-purpose counters
    uint32_t events_mask_len : 8; // Length of CPUID(0xa).EBX bit vector
    // ebx
    uint32_t no_core_cycle : 1;          // Core cycle event not available
    uint32_t no_instruction_retired : 1; // Instruction retired event not available
    uint32_t no_reference_cycles : 1;    // Reference cycles event not available
    uint32_t no_llc_reference : 1;       // LLC-reference event not available
    uint32_t no_llc_misses : 1;          // LLC-misses event not available
    uint32_t no_br_insn_retired : 1;     // Branch instruction retired event not available
    uint32_t no_br_misses_retired : 1;   // Branch mispredict retired event not available
    uint32_t no_topdown_slots : 1;       // Topdown slots event not available
    uint32_t no_backend_bound : 1;       // Topdown backend bound not available
    uint32_t no_bad_speculation : 1;     // Topdown bad speculation not available
    uint32_t no_frontend_bound : 1;      // Topdown frontend bound not available
    uint32_t no_retiring : 1;            // Topdown retiring not available
    uint32_t no_lbr_inserts : 1;         // LBR inserts not available
    uint32_t : 19;                       // Reserved
    // ecx
    uint32_t pmu_fcounters_bitmap : 32; // Fixed-function PMU counters support bitmap
    // edx
    uint32_t num_counters_fixed : 5;    // Number of fixed PMU counters
    uint32_t bitwidth_fixed : 8;        // Bitwidth of PMU fixed counters
    uint32_t : 2;                       // Reserved
    uint32_t anythread_deprecation : 1; // AnyThread mode deprecation
    uint32_t : 16;                      // Reserved
} PACKED leaf_0xa_0_t;

/*
 * Leaf 0xb
 * CPU extended topology v1
 */

typedef struct leaf_0xb_n
{
    // eax
    uint32_t x2apic_id_shift : 5; // Bit width of this level (previous levels inclusive)
    uint32_t : 27;                // Reserved
    // ebx
    uint32_t domain_lcpus_count : 16; // Logical CPUs count across all instances of this domain
    uint32_t : 16;                    // Reserved
    // ecx
    uint32_t domain_nr : 8;   // This domain level (subleaf ID)
    uint32_t domain_type : 8; // This domain type
    uint32_t : 16;            // Reserved
    // edx
    uint32_t x2apic_id : 32; // x2APIC ID of current logical CPU
} PACKED leaf_0xb_n_t;

#define LEAF_0xb_SUBLEAF_N_FIRST 0
#define LEAF_0xb_SUBLEAF_N_LAST  1

/*
 * Leaf 0xd
 * CPU extended state
 */

typedef struct leaf_0xd_0
{
    // eax
    uint32_t xcr0_x87 : 1;              // XCR0.X87
    uint32_t xcr0_sse : 1;              // XCR0.SSE
    uint32_t xcr0_avx : 1;              // XCR0.AVX
    uint32_t xcr0_mpx_bndregs : 1;      // XCR0.BNDREGS: MPX BND0-BND3 registers
    uint32_t xcr0_mpx_bndcsr : 1;       // XCR0.BNDCSR: MPX BNDCFGU/BNDSTATUS registers
    uint32_t xcr0_avx512_opmask : 1;    // XCR0.OPMASK: AVX-512 k0-k7 registers
    uint32_t xcr0_avx512_zmm_hi256 : 1; // XCR0.ZMM_Hi256: AVX-512 ZMM0->ZMM7/15 registers
    uint32_t xcr0_avx512_hi16_zmm : 1;  // XCR0.HI16_ZMM: AVX-512 ZMM16->ZMM31 registers
    uint32_t : 1;                       // Reserved
    uint32_t xcr0_pkru : 1;             // XCR0.PKRU: XSAVE PKRU registers
    uint32_t : 1;                       // Reserved
    uint32_t xcr0_cet_u : 1;            // XCR0.CET_U: CET user state
    uint32_t xcr0_cet_s : 1;            // XCR0.CET_S: CET supervisor state
    uint32_t : 4;                       // Reserved
    uint32_t xcr0_tileconfig : 1;       // XCR0.TILECONFIG: AMX can manage TILECONFIG
    uint32_t xcr0_tiledata : 1;         // XCR0.TILEDATA: AMX can manage TILEDATA
    uint32_t : 13;                      // Reserved
    // ebx
    uint32_t xsave_sz_xcr0 : 32; // XSAVE/XRSTOR area byte size, for XCR0 enabled features
    // ecx
    uint32_t xsave_sz_max : 32; // XSAVE/XRSTOR area max byte size, all CPU features
    // edx
    uint32_t : 30;         // Reserved
    uint32_t xcr0_lwp : 1; // AMD XCR0.LWP: Light-weight Profiling
    uint32_t : 1;          // Reserved
} PACKED leaf_0xd_0_t;

typedef struct leaf_0xd_1
{
    // eax
    uint32_t xsaveopt : 1; // XSAVEOPT instruction
    uint32_t xsavec : 1;   // XSAVEC instruction
    uint32_t xgetbv1 : 1;  // XGETBV instruction with ECX = 1
    uint32_t xsaves : 1;   // XSAVES/XRSTORS instructions (and XSS MSR)
    uint32_t xfd : 1;      // Extended feature disable
    uint32_t : 27;         // Reserved
    // ebx
    uint32_t xsave_sz_xcr0_xss : 32; // XSAVES/XSAVEC area byte size, for XCR0|XSS enabled features
    // ecx
    uint32_t : 8;           // Reserved
    uint32_t xss_pt : 1;    // PT state
    uint32_t : 1;           // Reserved
    uint32_t xss_pasid : 1; // PASID state
    uint32_t xss_cet_u : 1; // CET user state
    uint32_t xss_cet_s : 1; // CET supervisor state
    uint32_t xss_hdc : 1;   // HDC state
    uint32_t xss_uintr : 1; // UINTR state
    uint32_t xss_lbr : 1;   // LBR state
    uint32_t xss_hwp : 1;   // HWP state
    uint32_t : 15;          // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0xd_1_t;

typedef struct leaf_0xd_n
{
    // eax
    uint32_t xsave_sz : 32; // Subleaf-N feature save area size, in bytes
    // ebx
    uint32_t xsave_offset : 32; // Subleaf-N feature save area offset, in bytes
    // ecx
    uint32_t is_xss_bit : 1;                     // Subleaf N describes an XSS bit (otherwise XCR0)
    uint32_t compacted_xsave_64byte_aligned : 1; // When compacted, subleaf-N XSAVE area is 64-byte aligned
    uint32_t : 30;                               // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0xd_n_t;

#define LEAF_0xd_SUBLEAF_N_FIRST 2
#define LEAF_0xd_SUBLEAF_N_LAST  63

/*
 * Leaf 0xf
 * Intel RDT / AMD PQoS resource monitoring
 */

typedef struct leaf_0xf_0
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t core_rmid_max : 32; // RMID max within this core (0-based)
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 1;             // Reserved
    uint32_t llc_qos_mon : 1; // LLC QoS-monitoring
    uint32_t : 30;            // Reserved
} PACKED leaf_0xf_0_t;

typedef struct leaf_0xf_1
{
    // eax
    uint32_t l3c_qm_bitwidth : 8;     // L3 QoS-monitoring counter bitwidth (24-based)
    uint32_t l3c_qm_overflow_bit : 1; // QM_CTR MSR bit 61 is an overflow bit
    uint32_t io_rdt_cmt : 1;          // non-CPU agent supporting Intel RDT CMT present
    uint32_t io_rdt_mbm : 1;          // non-CPU agent supporting Intel RDT MBM present
    uint32_t : 21;                    // Reserved
    // ebx
    uint32_t l3c_qm_conver_factor : 32; // QM_CTR MSR conversion factor to bytes
    // ecx
    uint32_t l3c_qm_rmid_max : 32; // L3 QoS-monitoring max RMID
    // edx
    uint32_t l3c_qm_occupancy : 1; // L3 QoS occupancy monitoring
    uint32_t l3c_qm_mbm_total : 1; // L3 QoS total bandwidth monitoring
    uint32_t l3c_qm_mbm_local : 1; // L3 QoS local bandwidth monitoring
    uint32_t : 29;                 // Reserved
} PACKED leaf_0xf_1_t;

/*
 * Leaf 0x10
 * Intel RDT / AMD PQoS allocation
 */

typedef struct leaf_0x10_0
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t : 1;        // Reserved
    uint32_t cat_l3 : 1; // L3 Cache Allocation Technology
    uint32_t cat_l2 : 1; // L2 Cache Allocation Technology
    uint32_t mba : 1;    // Memory Bandwidth Allocation
    uint32_t : 28;       // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x10_0_t;

typedef struct leaf_0x10_n
{
    // eax
    uint32_t cat_cbm_len : 5; // L3/L2_CAT capacity bitmask length, minus-one notation
    uint32_t : 27;            // Reserved
    // ebx
    uint32_t cat_units_bitmap : 32; // L3/L2_CAT allocation units bitmap
    // ecx
    uint32_t : 1;                           // Reserved
    uint32_t l3_cat_cos_infreq_updates : 1; // L3_CAT COS updates should be infrequent
    uint32_t cat_cdp_supported : 1;         // L3/L2_CAT Code and Data Prioritization
    uint32_t cat_sparse_1s : 1;             // L3/L2_CAT non-contiguous 1s value
    uint32_t : 28;                          // Reserved
    // edx
    uint32_t cat_cos_max : 16; // L3/L2_CAT max Class of Service
    uint32_t : 16;             // Reserved
} PACKED leaf_0x10_n_t;

#define LEAF_0x10_SUBLEAF_N_FIRST 1
#define LEAF_0x10_SUBLEAF_N_LAST  2

typedef struct leaf_0x10_3
{
    // eax
    uint32_t mba_max_delay : 12; // Max MBA throttling value; minus-one notation
    uint32_t : 20;               // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t mba_per_thread : 1;   // Per-thread MBA controls
    uint32_t : 1;                  // Reserved
    uint32_t mba_delay_linear : 1; // Delay values are linear
    uint32_t : 29;                 // Reserved
    // edx
    uint32_t mba_cos_max : 16; // MBA max Class of Service
    uint32_t : 16;             // Reserved
} PACKED leaf_0x10_3_t;

/*
 * Leaf 0x12
 * Intel SGX (Software Guard Extensions)
 */

typedef struct leaf_0x12_0
{
    // eax
    uint32_t sgx1 : 1;                 // SGX1 leaf functions
    uint32_t sgx2 : 1;                 // SGX2 leaf functions
    uint32_t : 3;                      // Reserved
    uint32_t enclv_leaves : 1;         // ENCLV leaves
    uint32_t encls_leaves : 1;         // ENCLS leaves
    uint32_t enclu_everifyreport2 : 1; // ENCLU leaf EVERIFYREPORT2
    uint32_t : 2;                      // Reserved
    uint32_t encls_eupdatesvn : 1;     // ENCLS leaf EUPDATESVN
    uint32_t enclu_edeccssa : 1;       // ENCLU leaf EDECCSSA
    uint32_t : 20;                     // Reserved
    // ebx
    uint32_t miscselect_exinfo : 1; // SSA.MISC frame: Enclave #PF and #GP reporting
    uint32_t miscselect_cpinfo : 1; // SSA.MISC frame: Enclave #CP reporting
    uint32_t : 30;                  // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t max_enclave_sz_not64 : 8; // Maximum enclave size in non-64-bit mode (log2)
    uint32_t max_enclave_sz_64 : 8;    // Maximum enclave size in 64-bit mode (log2)
    uint32_t : 16;                     // Reserved
} PACKED leaf_0x12_0_t;

typedef struct leaf_0x12_1
{
    // eax
    uint32_t secs_attr_init : 1;           // Enclave initialized by EINIT
    uint32_t secs_attr_debug : 1;          // Enclave permits debugger read/write
    uint32_t secs_attr_mode64bit : 1;      // Enclave runs in 64-bit mode
    uint32_t : 1;                          // Reserved
    uint32_t secs_attr_provisionkey : 1;   // Provisioning key
    uint32_t secs_attr_einittoken_key : 1; // EINIT token key
    uint32_t secs_attr_cet : 1;            // CET attributes
    uint32_t secs_attr_kss : 1;            // Key Separation and Sharing
    uint32_t : 2;                          // Reserved
    uint32_t secs_attr_aexnotify : 1;      // Enclave threads: AEX notifications
    uint32_t : 21;                         // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t xfrm_x87 : 1;              // Enclave XFRM.X87
    uint32_t xfrm_sse : 1;              // Enclave XFRM.SSE
    uint32_t xfrm_avx : 1;              // Enclave XFRM.AVX
    uint32_t xfrm_mpx_bndregs : 1;      // Enclave XFRM.BNDREGS (MPX BND0-BND3 registers)
    uint32_t xfrm_mpx_bndcsr : 1;       // Enclave XFRM.BNDCSR (MPX BNDCFGU/BNDSTATUS registers)
    uint32_t xfrm_avx512_opmask : 1;    // Enclave XFRM.OPMASK (AVX-512 k0-k7 registers)
    uint32_t xfrm_avx512_zmm_hi256 : 1; // Enclave XFRM.ZMM_Hi256 (AVX-512 ZMM0->ZMM7/15 registers)
    uint32_t xfrm_avx512_hi16_zmm : 1;  // Enclave XFRM.HI16_ZMM (AVX-512 ZMM16->ZMM31 registers)
    uint32_t : 1;                       // Reserved
    uint32_t xfrm_pkru : 1;             // Enclave XFRM.PKRU (XSAVE PKRU registers)
    uint32_t : 7;                       // Reserved
    uint32_t xfrm_tileconfig : 1;       // Enclave XFRM.TILECONFIG (AMX can manage TILECONFIG)
    uint32_t xfrm_tiledata : 1;         // Enclave XFRM.TILEDATA (AMX can manage TILEDATA)
    uint32_t : 13;                      // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x12_1_t;

typedef struct leaf_0x12_n
{
    // eax
    uint32_t subleaf_type : 4;         // Subleaf type
    uint32_t : 8;                      // Reserved
    uint32_t epc_sec_base_addr_0 : 20; // EPC section base address, bits[12:31]
    // ebx
    uint32_t epc_sec_base_addr_1 : 20; // EPC section base address, bits[32:51]
    uint32_t : 12;                     // Reserved
    // ecx
    uint32_t epc_sec_type : 4;    // EPC section type / property encoding
    uint32_t : 8;                 // Reserved
    uint32_t epc_sec_size_0 : 20; // EPC section size, bits[12:31]
    // edx
    uint32_t epc_sec_size_1 : 20; // EPC section size, bits[32:51]
    uint32_t : 12;                // Reserved
} PACKED leaf_0x12_n_t;

#define LEAF_0x12_SUBLEAF_N_FIRST 2
#define LEAF_0x12_SUBLEAF_N_LAST  31

/*
 * Leaf 0x14
 * Intel Processor Trace
 */

typedef struct leaf_0x14_0
{
    // eax
    uint32_t pt_max_subleaf : 32; // Maximum leaf 0x14 subleaf
    // ebx
    uint32_t cr3_filtering : 1;     // IA32_RTIT_CR3_MATCH is accessible
    uint32_t psb_cyc : 1;           // Configurable PSB and cycle-accurate mode
    uint32_t ip_filtering : 1;      // IP/TraceStop filtering; Warm-reset PT MSRs preservation
    uint32_t mtc_timing : 1;        // MTC timing packet; COFI-based packets suppression
    uint32_t ptwrite : 1;           // PTWRITE instruction
    uint32_t power_event_trace : 1; // Power Event Trace
    uint32_t psb_pmi_preserve : 1;  // PSB and PMI preservation
    uint32_t event_trace : 1;       // Event Trace packet generation
    uint32_t tnt_disable : 1;       // TNT packet generation disable
    uint32_t : 23;                  // Reserved
    // ecx
    uint32_t topa_output : 1;            // ToPA output scheme
    uint32_t topa_multiple_entries : 1;  // ToPA tables can hold multiple entries
    uint32_t single_range_output : 1;    // Single-range output
    uint32_t trace_transport_output : 1; // Trace Transport subsystem output
    uint32_t : 27;                       // Reserved
    uint32_t ip_payloads_lip : 1;        // IP payloads have LIP values (CS base included)
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x14_0_t;

typedef struct leaf_0x14_1
{
    // eax
    uint32_t num_address_ranges : 3; // Number of configurable address ranges
    uint32_t : 13;                   // Reserved
    uint32_t mtc_periods_bmp : 16;   // MTC period encodings bitmap
    // ebx
    uint32_t cycle_thresholds_bmp : 16; // Cycle Threshold encodings bitmap
    uint32_t psb_periods_bmp : 16;      // Configurable PSB frequency encodings bitmap
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x14_1_t;

/*
 * Leaf 0x15
 * Intel TSC (Time Stamp Counter)
 */

typedef struct leaf_0x15_0
{
    // eax
    uint32_t tsc_denominator : 32; // Denominator of the TSC/'core crystal clock' ratio
    // ebx
    uint32_t tsc_numerator : 32; // Numerator of the TSC/'core crystal clock' ratio
    // ecx
    uint32_t cpu_crystal_hz : 32; // Core crystal clock nominal frequency, in Hz
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x15_0_t;

/*
 * Leaf 0x16
 * Intel processor frequency
 */

typedef struct leaf_0x16_0
{
    // eax
    uint32_t cpu_base_mhz : 16; // Processor base frequency, in MHz
    uint32_t : 16;              // Reserved
    // ebx
    uint32_t cpu_max_mhz : 16; // Processor max frequency, in MHz
    uint32_t : 16;             // Reserved
    // ecx
    uint32_t bus_mhz : 16; // Bus reference frequency, in MHz
    uint32_t : 16;         // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x16_0_t;

/*
 * Leaf 0x17
 * Intel SoC vendor attributes
 */

typedef struct leaf_0x17_0
{
    // eax
    uint32_t soc_max_subleaf : 32; // Maximum leaf 0x17 subleaf
    // ebx
    uint32_t soc_vendor_id : 16;   // SoC vendor ID
    uint32_t is_vendor_scheme : 1; // Assigned by industry enumeration scheme (not Intel)
    uint32_t : 15;                 // Reserved
    // ecx
    uint32_t soc_proj_id : 32; // SoC project ID, assigned by vendor
    // edx
    uint32_t soc_stepping_id : 32; // SoC project stepping ID, assigned by vendor
} PACKED leaf_0x17_0_t;

typedef struct leaf_0x17_n
{
    // eax
    uint32_t vendor_brand_a : 32; // Vendor Brand ID string, bytes subleaf_nr * (0 -> 3)
    // ebx
    uint32_t vendor_brand_b : 32; // Vendor Brand ID string, bytes subleaf_nr * (4 -> 7)
    // ecx
    uint32_t vendor_brand_c : 32; // Vendor Brand ID string, bytes subleaf_nr * (8 -> 11)
    // edx
    uint32_t vendor_brand_d : 32; // Vendor Brand ID string, bytes subleaf_nr * (12 -> 15)
} PACKED leaf_0x17_n_t;

#define LEAF_0x17_SUBLEAF_N_FIRST 1
#define LEAF_0x17_SUBLEAF_N_LAST  3

/*
 * Leaf 0x18
 * Intel deterministic address translation (TLB) parameters
 */

typedef struct leaf_0x18_n
{
    // eax
    uint32_t tlb_max_subleaf : 32; // Maximum leaf 0x18 subleaf
    // ebx
    uint32_t tlb_4k_page : 1;        // TLB supports 4KB-page entries
    uint32_t tlb_2m_page : 1;        // TLB supports 2MB-page entries
    uint32_t tlb_4m_page : 1;        // TLB supports 4MB-page entries
    uint32_t tlb_1g_page : 1;        // TLB supports 1GB-page entries
    uint32_t : 4;                    // Reserved
    uint32_t hard_partitioning : 3;  // Partitioning between logical CPUs
    uint32_t : 5;                    // Reserved
    uint32_t n_way_associative : 16; // Ways of associativity
    // ecx
    uint32_t n_sets : 32; // Number of sets
    // edx
    uint32_t tlb_type : 5;                 // Translation cache type (TLB type)
    uint32_t tlb_cache_level : 3;          // Translation cache level (1-based)
    uint32_t is_fully_associative : 1;     // Fully-associative
    uint32_t : 5;                          // Reserved
    uint32_t tlb_max_addressable_ids : 12; // Max number of addressable IDs - 1
    uint32_t : 6;                          // Reserved
} PACKED leaf_0x18_n_t;

#define LEAF_0x18_SUBLEAF_N_FIRST 0
#define LEAF_0x18_SUBLEAF_N_LAST  31

/*
 * Leaf 0x19
 * Intel key locker
 */

typedef struct leaf_0x19_0
{
    // eax
    uint32_t kl_cpl0_only : 1;  // CPL0-only key locker restriction
    uint32_t kl_no_encrypt : 1; // No-encrypt key locker restriction
    uint32_t kl_no_decrypt : 1; // No-decrypt key locker restriction
    uint32_t : 29;              // Reserved
    // ebx
    uint32_t aes_keylocker : 1;      // AES key locker instructions
    uint32_t : 1;                    // Reserved
    uint32_t aes_keylocker_wide : 1; // AES wide key locker instructions
    uint32_t : 1;                    // Reserved
    uint32_t kl_msr_iwkey : 1;       // Key locker MSRs and IWKEY backups
    uint32_t : 27;                   // Reserved
    // ecx
    uint32_t loadiwkey_no_backup : 1; // LOADIWKEY NoBackup parameter
    uint32_t iwkey_rand : 1;          // IWKEY randomization
    uint32_t : 30;                    // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x19_0_t;

/*
 * Leaf 0x1a
 * Intel hybrid CPUs identification (e.g. Atom, Core)
 */

typedef struct leaf_0x1a_0
{
    // eax
    uint32_t core_native_model : 24; // This core's native model ID
    uint32_t core_type : 8;          // This core's type
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x1a_0_t;

/*
 * Leaf 0x1b
 * Intel PCONFIG (Platform configuration)
 */

typedef struct leaf_0x1b_n
{
    // eax
    uint32_t pconfig_subleaf_type : 12; // CPUID 0x1b subleaf type
    uint32_t : 20;                      // Reserved
    // ebx
    uint32_t pconfig_target_id_x : 32; // A supported PCONFIG target ID
    // ecx
    uint32_t pconfig_target_id_y : 32; // A supported PCONFIG target ID
    // edx
    uint32_t pconfig_target_id_z : 32; // A supported PCONFIG target ID
} PACKED leaf_0x1b_n_t;

#define LEAF_0x1b_SUBLEAF_N_FIRST 0
#define LEAF_0x1b_SUBLEAF_N_LAST  31

/*
 * Leaf 0x1c
 * Intel LBR (Last Branch Record)
 */

typedef struct leaf_0x1c_0
{
    // eax
    uint32_t lbr_depth_mask : 8;   // Max LBR stack depth bitmask
    uint32_t : 22;                 // Reserved
    uint32_t lbr_deep_c_reset : 1; // LBRs may be cleared on MWAIT C-state > C1
    uint32_t lbr_ip_is_lip : 1;    // LBR IP contain Last IP (otherwise effective IP)
    // ebx
    uint32_t lbr_cpl : 1;           // CPL filtering
    uint32_t lbr_branch_filter : 1; // Branch filtering
    uint32_t lbr_call_stack : 1;    // Call-stack mode
    uint32_t : 29;                  // Reserved
    // ecx
    uint32_t lbr_mispredict : 1;     // Branch misprediction bit
    uint32_t lbr_timed_lbr : 1;      // Timed LBRs (CPU cycles since last LBR entry)
    uint32_t lbr_branch_type : 1;    // Branch type field
    uint32_t : 13;                   // Reserved
    uint32_t lbr_events_gpc_bmp : 4; // PMU-events logging support
    uint32_t : 12;                   // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x1c_0_t;

/*
 * Leaf 0x1d
 * Intel AMX (Advanced Matrix Extensions) tile information
 */

typedef struct leaf_0x1d_0
{
    // eax
    uint32_t amx_max_palette : 32; // Highest palette ID / subleaf ID
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x1d_0_t;

typedef struct leaf_0x1d_1
{
    // eax
    uint32_t amx_palette_size : 16; // AMX palette total tiles size, in bytes
    uint32_t amx_tile_size : 16;    // AMX single tile's size, in bytes
    // ebx
    uint32_t amx_tile_row_size : 16;    // AMX tile single row's size, in bytes
    uint32_t amx_palette_nr_tiles : 16; // AMX palette number of tiles
    // ecx
    uint32_t amx_tile_nr_rows : 16; // AMX tile max number of rows
    uint32_t : 16;                  // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x1d_1_t;

/*
 * Leaf 0x1e
 * Intel TMUL (Tile-matrix Multiply)
 */

typedef struct leaf_0x1e_0
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t tmul_maxk : 8;  // TMUL unit maximum height, K (rows or columns)
    uint32_t tmul_maxn : 16; // TMUL unit maximum SIMD dimension, N (column bytes)
    uint32_t : 8;            // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x1e_0_t;

/*
 * Leaf 0x1f
 * Intel extended topology v2
 */

typedef struct leaf_0x1f_n
{
    // eax
    uint32_t x2apic_id_shift : 5; // Bit width of this level (previous levels inclusive)
    uint32_t : 27;                // Reserved
    // ebx
    uint32_t domain_lcpus_count : 16; // Logical CPUs count across all instances of this domain
    uint32_t : 16;                    // Reserved
    // ecx
    uint32_t domain_level : 8; // This domain level (subleaf ID)
    uint32_t domain_type : 8;  // This domain type
    uint32_t : 16;             // Reserved
    // edx
    uint32_t x2apic_id : 32; // x2APIC ID of current logical CPU
} PACKED leaf_0x1f_n_t;

#define LEAF_0x1f_SUBLEAF_N_FIRST 0
#define LEAF_0x1f_SUBLEAF_N_LAST  5

/*
 * Leaf 0x20
 * Intel HRESET (History Reset)
 */

typedef struct leaf_0x20_0
{
    // eax
    uint32_t hreset_nr_subleaves : 32; // CPUID 0x20 max subleaf + 1
    // ebx
    uint32_t hreset_thread_director : 1; // Intel thread director HRESET
    uint32_t : 31;                       // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x20_0_t;

/*
 * Leaf 0x21
 * Intel TD (Trust Domain)
 */

typedef struct leaf_0x21_0
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t tdx_vendorid_0 : 32; // TDX vendor ID string bytes 0 - 3
    // ecx
    uint32_t tdx_vendorid_2 : 32; // TDX vendor ID string bytes 8 - 11
    // edx
    uint32_t tdx_vendorid_1 : 32; // TDX vendor ID string bytes 4 - 7
} PACKED leaf_0x21_0_t;

/*
 * Leaf 0x23
 * Intel Architectural Performance Monitoring Extended (ArchPerfmonExt)
 */

typedef struct leaf_0x23_0
{
    // eax
    uint32_t subleaf_0 : 1;         // Subleaf 0, this subleaf
    uint32_t counters_subleaf : 1;  // Subleaf 1, PMU counter bitmaps
    uint32_t acr_subleaf : 1;       // Subleaf 2, Auto Counter Reload bitmaps
    uint32_t events_subleaf : 1;    // Subleaf 3, PMU event bitmaps
    uint32_t pebs_caps_subleaf : 1; // Subleaf 4, PEBS capabilities
    uint32_t pebs_subleaf : 1;      // Subleaf 5, Arch PEBS bitmaps
    uint32_t : 26;                  // Reserved
    // ebx
    uint32_t unitmask2 : 1;          // IA32_PERFEVTSELx MSRs UnitMask2 bit
    uint32_t eq : 1;                 // IA32_PERFEVTSELx MSRs EQ bit
    uint32_t rdpmc_user_disable : 1; // RDPMC userspace disable
    uint32_t : 29;                   // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x23_0_t;

typedef struct leaf_0x23_1
{
    // eax
    uint32_t gp_counters : 32; // Bitmap of general-purpose PMU counters
    // ebx
    uint32_t fixed_counters : 32; // Bitmap of fixed PMU counters
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x23_1_t;

typedef struct leaf_0x23_2
{
    // eax
    uint32_t acr_gp_reload : 32; // Bitmap of general-purpose counters that can be reloaded
    // ebx
    uint32_t acr_fixed_reload : 32; // Bitmap of fixed counters that can be reloaded
    // ecx
    uint32_t acr_gp_trigger : 32; // Bitmap of general-purpose counters that can trigger reloads
    // edx
    uint32_t acr_fixed_trigger : 32; // Bitmap of fixed counters that can trigger reloads
} PACKED leaf_0x23_2_t;

typedef struct leaf_0x23_3
{
    // eax
    uint32_t core_cycles_evt : 1;       // Core cycles event
    uint32_t insn_retired_evt : 1;      // Instructions retired event
    uint32_t ref_cycles_evt : 1;        // Reference cycles event
    uint32_t llc_refs_evt : 1;          // Last-level cache references event
    uint32_t llc_misses_evt : 1;        // Last-level cache misses event
    uint32_t br_insn_ret_evt : 1;       // Branch instruction retired event
    uint32_t br_mispr_evt : 1;          // Branch mispredict retired event
    uint32_t td_slots_evt : 1;          // Topdown slots event
    uint32_t td_backend_bound_evt : 1;  // Topdown backend bound event
    uint32_t td_bad_spec_evt : 1;       // Topdown bad speculation event
    uint32_t td_frontend_bound_evt : 1; // Topdown frontend bound event
    uint32_t td_retiring_evt : 1;       // Topdown retiring event
    uint32_t : 20;                      // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x23_3_t;

typedef struct leaf_0x23_4
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t : 3;                  // Reserved
    uint32_t allow_in_record : 1;  // ALLOW_IN_RECORD bit in MSRs
    uint32_t counters_gp : 1;      // Counters group sub-group general-purpose counters
    uint32_t counters_fixed : 1;   // Counters group sub-group fixed-function counters
    uint32_t counters_metrics : 1; // Counters group sub-group performance metrics
    uint32_t : 1;                  // Reserved
    uint32_t lbr : 2;              // LBR group
    uint32_t : 6;                  // Reserved
    uint32_t xer : 8;              // XER group
    uint32_t : 5;                  // Reserved
    uint32_t gpr : 1;              // GPR group
    uint32_t aux : 1;              // AUX group
    uint32_t : 1;                  // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x23_4_t;

typedef struct leaf_0x23_5
{
    // eax
    uint32_t pebs_gp : 32; // Architectural PEBS general-purpose counters
    // ebx
    uint32_t pebs_pdist_gp : 32; // Architectural PEBS PDIST general-purpose counters
    // ecx
    uint32_t pebs_fixed : 32; // Architectural PEBS fixed counters
    // edx
    uint32_t pebs_pdist_fixed : 32; // Architectural PEBS PDIST fixed counters
} PACKED leaf_0x23_5_t;

/*
 * Leaf 0x40000000
 * Maximum hypervisor leaf + hypervisor vendor string
 */

typedef struct leaf_0x40000000_0
{
    // eax
    uint32_t max_hyp_leaf : 32; // Maximum hypervisor leaf
    // ebx
    uint32_t hypervisor_id_0 : 32; // Hypervisor ID string bytes 0 - 3
    // ecx
    uint32_t hypervisor_id_1 : 32; // Hypervisor ID string bytes 4 - 7
    // edx
    uint32_t hypervisor_id_2 : 32; // Hypervisor ID string bytes 8 - 11
} PACKED leaf_0x40000000_0_t;

/*
 * Leaf 0x4c780001
 * Linux-defined synthetic feature flags
 */

typedef struct leaf_0x4c780001_0
{
    // eax
    uint32_t cxmmx : 1;          // Cyrix MMX extensions
    uint32_t k6_mtrr : 1;        // AMD K6 nonstandard MTRRs
    uint32_t cyrix_arr : 1;      // Cyrix ARRs (= MTRRs)
    uint32_t centaur_mcr : 1;    // Centaur MCRs (= MTRRs)
    uint32_t k8 : 1;             // Opteron, Athlon64
    uint32_t zen5 : 1;           // CPU based on Zen5 micro-architecture
    uint32_t zen6 : 1;           // CPU based on Zen6 micro-architecture
    uint32_t : 1;                // Reserved
    uint32_t constant_tsc : 1;   // TSC ticks at a constant rate
    uint32_t up : 1;             // SMP kernel running on UP
    uint32_t art : 1;            // Always running timer (ART)
    uint32_t arch_perfmon : 1;   // Intel Architectural PerfMon
    uint32_t pebs : 1;           // Precise-Event Based Sampling
    uint32_t bts : 1;            // Branch Trace Store
    uint32_t syscall32 : 1;      // SYSCALL in IA32 userspace
    uint32_t sysenter32 : 1;     // SYSENTER in IA32 userspace
    uint32_t rep_good : 1;       // REP microcode works well
    uint32_t amd_lbr_v2 : 1;     // AMD Last Branch Record Extension version 2
    uint32_t clear_cpu_buf : 1;  // Clear CPU buffers using VERW
    uint32_t acc_power : 1;      // AMD Accumulated Power Mechanism
    uint32_t nopl : 1;           // The NOPL instructions
    uint32_t always : 1;         // Always-present feature
    uint32_t xtopology : 1;      // CPU topology enumeration extensions
    uint32_t tsc_reliable : 1;   // TSC is known to be reliable
    uint32_t nonstop_tsc : 1;    // TSC does not stop in C states
    uint32_t cpuid : 1;          // CPU has the CPUID instruction
    uint32_t extd_apicid : 1;    // Extended APIC ID (8 bits)
    uint32_t amd_dcm : 1;        // AMD multi-node processor
    uint32_t aperfmperf : 1;     // APERF/MPERF MSRs: P-State hardware coordination feedback
    uint32_t rapl : 1;           // AMD/Hygon RAPL interface
    uint32_t nonstop_tsc_s3 : 1; // TSC does not stop in S3 state
    uint32_t tsc_known_freq : 1; // TSC has known frequency
    // ebx
    uint32_t ring3mwait : 1;        // Ring 3 MONITOR/MWAIT instructions
    uint32_t cpuid_fault : 1;       // Intel CPUID faulting
    uint32_t cpb : 1;               // AMD Core Performance Boost
    uint32_t epb : 1;               // IA32_ENERGY_PERF_BIAS support
    uint32_t cat_l3 : 1;            // Cache Allocation Technology L3
    uint32_t cat_l2 : 1;            // Cache Allocation Technology L2
    uint32_t cdp_l3 : 1;            // Code and Data Prioritization L3
    uint32_t tdx_host_platform : 1; // Platform supports being a TDX host
    uint32_t hw_pstate : 1;         // AMD Hardware P-state control
    uint32_t proc_feedback : 1;     // AMD Processor Feedback Interface
    uint32_t xcompacted : 1;        // Use compacted XSTATE (XSAVES or XSAVEC)
    uint32_t pti : 1;               // Kernel Page Table Isolation enabled
    uint32_t kernel_ibrs : 1;       // Set/clear IBRS on kernel entry/exit
    uint32_t rsb_vmexit : 1;        // Fill RSB on VM-Exit
    uint32_t intel_ppin : 1;        // Intel Processor Inventory Number
    uint32_t cdp_l2 : 1;            // Code and Data Prioritization L2
    uint32_t msr_spec_ctrl : 1;     // MSR SPEC_CTRL is implemented
    uint32_t ssbd : 1;              // Speculative Store Bypass Disable
    uint32_t mba : 1;               // Memory Bandwidth Allocation
    uint32_t rsb_ctxsw : 1;         // Fill RSB on context switches
    uint32_t perfmon_v2 : 1;        // AMD Performance Monitoring Version 2
    uint32_t : 1;                   // Reserved
    uint32_t use_ibrs_fw : 1;       // Use IBRS during runtime firmware calls
    uint32_t ss_bypass_disable : 1; // Disable Speculative Store Bypass
    uint32_t ls_cfg_ssbd : 1;       // AMD SSBD implementation via LS_CFG MSR
    uint32_t ibrs : 1;              // Indirect Branch Restricted Speculation
    uint32_t ibpb : 1;              // Indirect Branch Prediction Barrier (without RSB flush guarantee)
    uint32_t stibp : 1;             // Single Thread Indirect Branch Predictors
    uint32_t zen : 1;               // Generic flag for all Zen and newer
    uint32_t l1tf_pteinv : 1;       // L1TF workaround PTE inversion
    uint32_t ibrs_enhanced : 1;     // Enhanced IBRS
    uint32_t msr_ia32_feat_ctl : 1; // MSR IA32_FEAT_CTL configured
    // ecx
    uint32_t tpr_shadow : 1;       // Intel TPR Shadow
    uint32_t flexpriority : 1;     // Intel FlexPriority
    uint32_t ept : 1;              // Intel Extended Page Table
    uint32_t vpid : 1;             // Intel Virtual Processor ID
    uint32_t coherency_sfw_no : 1; // SNP cache coherency software workaround not needed
    uint32_t : 10;                 // Reserved
    uint32_t vmmcall : 1;          // Prefer VMMCALL to VMCALL
    uint32_t xenpv : 1;            // Xen paravirtual guest
    uint32_t ept_ad : 1;           // Intel Extended Page Table access-dirty bit
    uint32_t vmcall : 1;           // Hypervisor supports the VMCALL instruction
    uint32_t vmw_vmmcall : 1;      // VMware prefers the VMMCALL instruction
    uint32_t pvunlock : 1;         // PV unlock function
    uint32_t vcpupreempt : 1;      // PV vcpu_is_preempted function
    uint32_t tdx_guest : 1;        // Intel Trust Domain Extensions Guest
    uint32_t : 9;                  // Reserved
    // edx
    uint32_t cqm_llc : 1;             // LLC QoS
    uint32_t cqm_occup_llc : 1;       // LLC occupancy monitoring
    uint32_t cqm_mbm_total : 1;       // LLC Total MBM monitoring
    uint32_t cqm_mbm_local : 1;       // LLC Local MBM monitoring
    uint32_t fence_swapgs_user : 1;   // LFENCE in user entry SWAPGS path
    uint32_t fence_swapgs_kernel : 1; // LFENCE in kernel entry SWAPGS path
    uint32_t split_lock_detect : 1;   // #AC for split lock
    uint32_t per_thread_mba : 1;      // Per-thread Memory Bandwidth Allocation
    uint32_t sgx1 : 1;                // SGX Basic
    uint32_t sgx2 : 1;                // SGX Enclave Dynamic Memory Management (EDMM)
    uint32_t entry_ibpb : 1;          // Issue an IBPB on kernel entry
    uint32_t rrsba_ctrl : 1;          // RET prediction control
    uint32_t retpoline : 1;           // Generic Retpoline mitigation for Spectre variant 2
    uint32_t retpoline_lfence : 1;    // Use LFENCE for Spectre variant 2
    uint32_t rethunk : 1;             // Use Return THUNK
    uint32_t unret : 1;               // AMD BTB untrain return
    uint32_t use_ibpb_fw : 1;         // Use IBPB during runtime firmware calls
    uint32_t rsb_vmexit_lite : 1;     // Fill RSB on VM exit when EIBRS is enabled
    uint32_t sgx_edeccssa : 1;        // SGX EDECCSSA user leaf function
    uint32_t call_depth : 1;          // Call depth tracking for RSB stuffing
    uint32_t msr_tsx_ctrl : 1;        // MSR IA32_TSX_CTRL (Intel) implemented
    uint32_t smba : 1;                // Slow Memory Bandwidth Allocation
    uint32_t bmec : 1;                // Bandwidth Monitoring Event Configuration
    uint32_t user_shstk : 1;          // Shadow stack support for user mode applications
    uint32_t srso : 1;                // AMD BTB untrain RETs
    uint32_t srso_alias : 1;          // AMD BTB untrain RETs through aliasing
    uint32_t ibpb_on_vmexit : 1;      // Issue an IBPB only on VMEXIT
    uint32_t apic_msrs_fence : 1;     // IA32_TSC_DEADLINE and X2APIC MSRs need fencing
    uint32_t zen2 : 1;                // CPU based on Zen2 microarchitecture
    uint32_t zen3 : 1;                // CPU based on Zen3 microarchitecture
    uint32_t zen4 : 1;                // CPU based on Zen4 microarchitecture
    uint32_t zen1 : 1;                // CPU based on Zen1 microarchitecture
} PACKED leaf_0x4c780001_0_t;

typedef struct leaf_0x4c780001_1
{
    // eax
    uint32_t overflow_recov : 1; // MCA overflow recovery support
    uint32_t succor : 1;         // Uncorrectable error containment and recovery
    uint32_t : 1;                // Reserved
    uint32_t smca : 1;           // Scalable MCA
    uint32_t : 28;               // Reserved
    // ebx
    uint32_t amd_lbr_pmc_freeze : 1; // AMD LBR and PMC Freeze
    uint32_t clear_bhb_loop : 1;     // Clear branch history at SYSCALL entry using SW loop
    uint32_t bhi_ctrl : 1;           // BHI_DIS_S HW control available
    uint32_t clear_bhb_hw : 1;       // BHI_DIS_S HW control enabled
    uint32_t clear_bhb_vmexit : 1;   // Clear branch history at VMEXIT using SW loop
    uint32_t amd_fast_cppc : 1;      // AMD fast Collaborative Processor Performance Control
    uint32_t amd_htr_cores : 1;      // Heterogeneous Core Topology
    uint32_t amd_workload_class : 1; // Workload Classification
    uint32_t prefer_ymm : 1;         // Avoid ZMM registers due to downclocking
    uint32_t apx : 1;                // Advanced Performance Extensions
    uint32_t indirect_thunk_its : 1; // Use thunk for indirect branches in lower half of cache line
    uint32_t tsa_sq_no : 1;          // AMD CPU not vulnerable to TSA-SQ
    uint32_t tsa_l1_no : 1;          // AMD CPU not vulnerable to TSA-L1
    uint32_t clear_cpu_buf_vm : 1;   // Clear CPU buffers using VERW before VMRUN
    uint32_t ibpb_exit_to_user : 1;  // Use IBPB on exit-to-userspace, see VMSCAPE bug
    uint32_t : 17;                   // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x4c780001_1_t;

/*
 * Leaf 0x4c780002
 * Linux-defined synthetic CPU bug flags
 */

typedef struct leaf_0x4c780002_0
{
    // eax
    uint32_t f00f : 1;              // Intel F00F
    uint32_t fdiv : 1;              // FPU FDIV
    uint32_t coma : 1;              // Cyrix 6x86 coma
    uint32_t amd_tlb_mmatch : 1;    // AMD Erratum 383
    uint32_t amd_apic_c1e : 1;      // AMD Erratum 400
    uint32_t bug_11ap : 1;          // Bad local APIC aka 11AP
    uint32_t fxsave_leak : 1;       // FXSAVE leaks FOP/FIP/FOP
    uint32_t clflush_monitor : 1;   // AAI65, CLFLUSH required before MONITOR
    uint32_t sysret_ss_attrs : 1;   // SYSRET does not fix up SS attributes
    uint32_t espfix : 1;            // IRET to 16-bit SS corrupts ESP/RSP high bits (x86-32)
    uint32_t null_seg : 1;          // Setting a selector to NULL preserves the base
    uint32_t swapgs_fence : 1;      // SWAPGS without input dep on GS
    uint32_t monitor : 1;           // IPI required to wake up remote CPU
    uint32_t amd_e400 : 1;          // CPU is among the affected by Erratum 400
    uint32_t cpu_meltdown : 1;      // CPU affected by meltdown; needs kernel page table isolation
    uint32_t spectre_v1 : 1;        // CPU affected by Spectre variant 1 with conditional branches
    uint32_t spectre_v2 : 1;        // CPU affected by Spectre variant 2 with indirect branches
    uint32_t spec_store_bypass : 1; // CPU affected by speculative store bypass attack
    uint32_t l1tf : 1;              // CPU affected by L1 Terminal Fault
    uint32_t mds : 1;               // CPU affected by Microarchitectural data sampling
    uint32_t msbds_only : 1;        // Microarchitectural data sampling: CPU only affected by the MSBDS variant
    uint32_t swapgs : 1;            // CPU affected by speculation through SWAPGS
    uint32_t taa : 1;               // CPU is affected by TSX Async Abort (TAA)
    uint32_t itlb_multihit : 1;     // CPU may incur MCE during certain page attribute changes
    uint32_t srbds : 1;             // CPU may leak RNG bits if not mitigated
    uint32_t mmio_stale_data : 1;   // CPU affected by Processor MMIO Stale Data vulnerabilities
    uint32_t : 1;                   // Reserved
    uint32_t retbleed : 1;          // CPU affected by Retbleed
    uint32_t eibrs_pbrsb : 1;       // EIBRS is vulnerable to Post Barrier RSB Predictions
    uint32_t smt_rsb : 1;           // CPU vulnerable to Cross-Thread Return Address Predictions
    uint32_t gds : 1;               // CPU affected by Gather Data Sampling
    uint32_t tdx_pw_mce : 1;        // CPU may incur #MC if non-TD software does partial write to TDX private memory
    // ebx
    uint32_t srso : 1;            // AMD SRSO bug
    uint32_t div0 : 1;            // AMD DIV0 speculation bug
    uint32_t rfds : 1;            // CPU vulnerable to Register File Data Sampling
    uint32_t bhi : 1;             // CPU affected by Branch History Injection
    uint32_t ibpb_no_ret : 1;     // IBPB omits return target predictions
    uint32_t spectre_v2_user : 1; // CPU affected by Spectre variant 2 between user processes
    uint32_t old_microcode : 1;   // CPU has old microcode; it must be vulnerable to something
    uint32_t its : 1;             // CPU affected by Indirect Target Selection
    uint32_t its_native_only : 1; // CPU affected by ITS; VMX is not affected
    uint32_t tsa : 1;             // CPU affected by Transient Scheduler Attacks
    uint32_t vmscape : 1;         // CPU affected by VMSCAPE attacks from guests
    uint32_t : 21;                // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x4c780002_0_t;

/*
 * Leaf 0x80000000
 * Maximum extended leaf + CPU vendor string
 */

typedef struct leaf_0x80000000_0
{
    // eax
    uint32_t max_ext_leaf : 32; // Maximum extended CPUID leaf
    // ebx
    uint32_t cpu_vendorid_0 : 32; // Vendor ID string bytes 0 - 3
    // ecx
    uint32_t cpu_vendorid_2 : 32; // Vendor ID string bytes 8 - 11
    // edx
    uint32_t cpu_vendorid_1 : 32; // Vendor ID string bytes 4 - 7
} PACKED leaf_0x80000000_0_t;

/*
 * Leaf 0x80000001
 * Extended CPU features
 */

typedef struct leaf_0x80000001_0
{
    // eax
    uint32_t e_stepping_id : 4; // Stepping ID
    uint32_t e_base_model : 4;  // Base processor model
    uint32_t e_base_family : 4; // Base processor family
    uint32_t e_base_type : 2;   // Base processor type (Transmeta)
    uint32_t : 2;               // Reserved
    uint32_t e_ext_model : 4;   // Extended processor model
    uint32_t e_ext_family : 8;  // Extended processor family
    uint32_t : 4;               // Reserved
    // ebx
    uint32_t brand_id : 16; // Brand ID
    uint32_t : 12;          // Reserved
    uint32_t pkg_type : 4;  // Package type
    // ecx
    uint32_t lahf_lm : 1;         // LAHF and SAHF in 64-bit mode
    uint32_t cmp_legacy : 1;      // Multi-processing legacy mode (No HT)
    uint32_t svm : 1;             // Secure Virtual Machine
    uint32_t extapic : 1;         // Extended APIC space
    uint32_t cr8_legacy : 1;      // LOCK MOV CR0 means MOV CR8
    uint32_t lzcnt_abm : 1;       // LZCNT advanced bit manipulation
    uint32_t sse4a : 1;           // SSE4A support
    uint32_t misaligned_sse : 1;  // Misaligned SSE mode
    uint32_t _3dnow_prefetch : 1; // 3DNow PREFETCH/PREFETCHW support
    uint32_t osvw : 1;            // OS visible workaround
    uint32_t ibs : 1;             // Instruction based sampling
    uint32_t xop : 1;             // XOP: extended operation (AVX instructions)
    uint32_t skinit : 1;          // SKINIT/STGI support
    uint32_t wdt : 1;             // Watchdog timer support
    uint32_t : 1;                 // Reserved
    uint32_t lwp : 1;             // Lightweight profiling
    uint32_t fma4 : 1;            // 4-operand FMA instruction
    uint32_t tce : 1;             // Translation cache extension
    uint32_t : 1;                 // Reserved
    uint32_t nodeid_msr : 1;      // NodeId MSR (0xc001100c)
    uint32_t : 1;                 // Reserved
    uint32_t tbm : 1;             // Trailing bit manipulations
    uint32_t topoext : 1;         // Topology Extensions (leaf 0x8000001d)
    uint32_t perfctr_core : 1;    // Core performance counter extensions
    uint32_t perfctr_nb : 1;      // NB/DF performance counter extensions
    uint32_t : 1;                 // Reserved
    uint32_t data_bp_ext : 1;     // Data access breakpoint extension
    uint32_t perf_tsc : 1;        // Performance time-stamp counter
    uint32_t perfctr_llc : 1;     // LLC (L3) performance counter extensions
    uint32_t mwaitx : 1;          // MWAITX/MONITORX support
    uint32_t addr_mask_ext : 1;   // Breakpoint address mask extension (to bit 31)
    uint32_t : 1;                 // Reserved
    // edx
    uint32_t e_fpu : 1;           // Floating-Point Unit on-chip (x87)
    uint32_t e_vme : 1;           // Virtual-8086 Mode Extensions
    uint32_t e_de : 1;            // Debugging Extensions
    uint32_t e_pse : 1;           // Page Size Extension
    uint32_t e_tsc : 1;           // Time Stamp Counter
    uint32_t e_msr : 1;           // Model-Specific Registers (RDMSR and WRMSR support)
    uint32_t pae : 1;             // Physical Address Extensions
    uint32_t mce : 1;             // Machine Check Exception
    uint32_t cx8 : 1;             // CMPXCHG8B instruction
    uint32_t apic : 1;            // APIC on-chip
    uint32_t : 1;                 // Reserved
    uint32_t syscall : 1;         // SYSCALL and SYSRET instructions
    uint32_t mtrr : 1;            // Memory Type Range Registers
    uint32_t pge : 1;             // Page Global Extensions
    uint32_t mca : 1;             // Machine Check Architecture
    uint32_t cmov : 1;            // Conditional Move Instruction
    uint32_t pat : 1;             // Page Attribute Table
    uint32_t pse36 : 1;           // Page Size Extension (36-bit)
    uint32_t : 1;                 // Reserved
    uint32_t obsolete_mp_bit : 1; // Out-of-spec AMD Multiprocessing bit
    uint32_t nx : 1;              // No-execute page protection
    uint32_t : 1;                 // Reserved
    uint32_t mmxext : 1;          // AMD MMX extensions
    uint32_t e_mmx : 1;           // MMX instructions
    uint32_t e_fxsr : 1;          // FXSAVE and FXRSTOR instructions
    uint32_t fxsr_opt : 1;        // FXSAVE and FXRSTOR optimizations
    uint32_t page1gb : 1;         // 1-GB large page support
    uint32_t rdtscp : 1;          // RDTSCP instruction
    uint32_t : 1;                 // Reserved
    uint32_t lm : 1;              // Long mode (x86-64, 64-bit support)
    uint32_t _3dnowext : 1;       // AMD 3DNow extensions
    uint32_t _3dnow : 1;          // 3DNow instructions
} PACKED leaf_0x80000001_0_t;

/*
 * Leaf 0x80000002
 * CPU brand ID string, bytes 0 - 15
 */

typedef struct leaf_0x80000002_0
{
    // eax
    uint32_t cpu_brandid_0 : 32; // CPU brand ID string, bytes 0 - 3
    // ebx
    uint32_t cpu_brandid_1 : 32; // CPU brand ID string, bytes 4 - 7
    // ecx
    uint32_t cpu_brandid_2 : 32; // CPU brand ID string, bytes 8 - 11
    // edx
    uint32_t cpu_brandid_3 : 32; // CPU brand ID string, bytes 12 - 15
} PACKED leaf_0x80000002_0_t;

/*
 * Leaf 0x80000003
 * CPU brand ID string, bytes 16 - 31
 */

typedef struct leaf_0x80000003_0
{
    // eax
    uint32_t cpu_brandid_4 : 32; // CPU brand ID string bytes, 16 - 19
    // ebx
    uint32_t cpu_brandid_5 : 32; // CPU brand ID string bytes, 20 - 23
    // ecx
    uint32_t cpu_brandid_6 : 32; // CPU brand ID string bytes, 24 - 27
    // edx
    uint32_t cpu_brandid_7 : 32; // CPU brand ID string bytes, 28 - 31
} PACKED leaf_0x80000003_0_t;

/*
 * Leaf 0x80000004
 * CPU brand ID string, bytes 32 - 47
 */

typedef struct leaf_0x80000004_0
{
    // eax
    uint32_t cpu_brandid_8 : 32; // CPU brand ID string, bytes 32 - 35
    // ebx
    uint32_t cpu_brandid_9 : 32; // CPU brand ID string, bytes 36 - 39
    // ecx
    uint32_t cpu_brandid_10 : 32; // CPU brand ID string, bytes 40 - 43
    // edx
    uint32_t cpu_brandid_11 : 32; // CPU brand ID string, bytes 44 - 47
} PACKED leaf_0x80000004_0_t;

/*
 * Leaf 0x80000005
 * AMD/Transmeta L1 cache and TLB
 */

typedef struct leaf_0x80000005_0
{
    // eax
    uint32_t l1_itlb_2m_4m_nentries : 8; // L1 ITLB #entries, 2M and 4M pages
    uint32_t l1_itlb_2m_4m_assoc : 8;    // L1 ITLB associativity, 2M and 4M pages
    uint32_t l1_dtlb_2m_4m_nentries : 8; // L1 DTLB #entries, 2M and 4M pages
    uint32_t l1_dtlb_2m_4m_assoc : 8;    // L1 DTLB associativity, 2M and 4M pages
    // ebx
    uint32_t l1_itlb_4k_nentries : 8; // L1 ITLB #entries, 4K pages
    uint32_t l1_itlb_4k_assoc : 8;    // L1 ITLB associativity, 4K pages
    uint32_t l1_dtlb_4k_nentries : 8; // L1 DTLB #entries, 4K pages
    uint32_t l1_dtlb_4k_assoc : 8;    // L1 DTLB associativity, 4K pages
    // ecx
    uint32_t l1_dcache_line_size : 8; // L1 dcache line size, in bytes
    uint32_t l1_dcache_nlines : 8;    // L1 dcache lines per tag
    uint32_t l1_dcache_assoc : 8;     // L1 dcache associativity
    uint32_t l1_dcache_size_kb : 8;   // L1 dcache size, in KB
    // edx
    uint32_t l1_icache_line_size : 8; // L1 icache line size, in bytes
    uint32_t l1_icache_nlines : 8;    // L1 icache lines per tag
    uint32_t l1_icache_assoc : 8;     // L1 icache associativity
    uint32_t l1_icache_size_kb : 8;   // L1 icache size, in KB
} PACKED leaf_0x80000005_0_t;

/*
 * Leaf 0x80000006
 * (Mostly AMD) L2/L3 cache and TLB
 */

typedef struct leaf_0x80000006_0
{
    // eax
    uint32_t l2_itlb_2m_4m_nentries : 12; // L2 iTLB #entries, 2M and 4M pages
    uint32_t l2_itlb_2m_4m_assoc : 4;     // L2 iTLB associativity, 2M and 4M pages
    uint32_t l2_dtlb_2m_4m_nentries : 12; // L2 dTLB #entries, 2M and 4M pages
    uint32_t l2_dtlb_2m_4m_assoc : 4;     // L2 dTLB associativity, 2M and 4M pages
    // ebx
    uint32_t l2_itlb_4k_nentries : 12; // L2 iTLB #entries, 4K pages
    uint32_t l2_itlb_4k_assoc : 4;     // L2 iTLB associativity, 4K pages
    uint32_t l2_dtlb_4k_nentries : 12; // L2 dTLB #entries, 4K pages
    uint32_t l2_dtlb_4k_assoc : 4;     // L2 dTLB associativity, 4K pages
    // ecx
    uint32_t l2_line_size : 8; // L2 cache line size, in bytes
    uint32_t l2_nlines : 4;    // L2 cache number of lines per tag
    uint32_t l2_assoc : 4;     // L2 cache associativity
    uint32_t l2_size_kb : 16;  // L2 cache size, in KB
    // edx
    uint32_t l3_line_size : 8;   // L3 cache line size, in bytes
    uint32_t l3_nlines : 4;      // L3 cache number of lines per tag
    uint32_t l3_assoc : 4;       // L3 cache associativity
    uint32_t : 2;                // Reserved
    uint32_t l3_size_range : 14; // L3 cache size range
} PACKED leaf_0x80000006_0_t;

/*
 * Leaf 0x80000007
 * CPU power management (mostly AMD) and AMD RAS
 */

typedef struct leaf_0x80000007_0
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t mca_overflow_recovery : 1; // MCA overflow conditions not fatal
    uint32_t succor : 1;                // Software containment of uncorrectable errors
    uint32_t hw_assert : 1;             // Hardware assert MSRs
    uint32_t scalable_mca : 1;          // Scalable MCA (MCAX MSRs)
    uint32_t : 28;                      // Reserved
    // ecx
    uint32_t cpu_pwr_sample_ratio : 32; // CPU power sample time ratio
    // edx
    uint32_t digital_temp : 1;         // Digital temperature sensor
    uint32_t powernow_freq_id : 1;     // PowerNOW! frequency scaling
    uint32_t powernow_volt_id : 1;     // PowerNOW! voltage scaling
    uint32_t thermal_trip : 1;         // THERMTRIP (Thermal Trip)
    uint32_t hw_thermal_control : 1;   // Hardware thermal control
    uint32_t sw_thermal_control : 1;   // Software thermal control
    uint32_t _100mhz_steps : 1;        // 100 MHz multiplier control
    uint32_t hw_pstate : 1;            // Hardware P-state control
    uint32_t constant_tsc : 1;         // TSC ticks at constant rate across all P and C states
    uint32_t core_perf_boost : 1;      // Core performance boost
    uint32_t eff_freq_ro : 1;          // Read-only effective frequency interface
    uint32_t proc_feedback : 1;        // Processor feedback interface (deprecated)
    uint32_t proc_power_reporting : 1; // Processor power reporting interface
    uint32_t connected_standby : 1;    // CPU Connected Standby support
    uint32_t rapl_interface : 1;       // Runtime Average Power Limit interface
    uint32_t : 17;                     // Reserved
} PACKED leaf_0x80000007_0_t;

/*
 * Leaf 0x80000008
 * CPU capacity parameters and extended feature flags (mostly AMD)
 */

typedef struct leaf_0x80000008_0
{
    // eax
    uint32_t phys_addr_bits : 8;       // Max physical address bits
    uint32_t virt_addr_bits : 8;       // Max virtual address bits
    uint32_t guest_phys_addr_bits : 8; // Max nested-paging guest physical address bits
    uint32_t : 8;                      // Reserved
    // ebx
    uint32_t clzero : 1;            // CLZERO instruction
    uint32_t insn_retired_perf : 1; // Instruction retired counter MSR
    uint32_t xsave_err_ptr : 1;     // XSAVE/XRSTOR always saves/restores FPU error pointers
    uint32_t invlpgb : 1;           // INVLPGB broadcasts a TLB invalidate
    uint32_t rdpru : 1;             // RDPRU (Read Processor Register at User level)
    uint32_t : 1;                   // Reserved
    uint32_t mba : 1;               // Memory Bandwidth Allocation (AMD bit)
    uint32_t : 1;                   // Reserved
    uint32_t mcommit : 1;           // MCOMMIT instruction
    uint32_t wbnoinvd : 1;          // WBNOINVD instruction
    uint32_t : 2;                   // Reserved
    uint32_t ibpb : 1;              // Indirect Branch Prediction Barrier
    uint32_t wbinvd_int : 1;        // Interruptible WBINVD/WBNOINVD
    uint32_t ibrs : 1;              // Indirect Branch Restricted Speculation
    uint32_t stibp : 1;             // Single Thread Indirect Branch Prediction mode
    uint32_t ibrs_always_on : 1;    // IBRS always-on preferred
    uint32_t stibp_always_on : 1;   // STIBP always-on preferred
    uint32_t ibrs_fast : 1;         // IBRS is preferred over software solution
    uint32_t ibrs_same_mode : 1;    // IBRS provides same mode protection
    uint32_t no_efer_lmsle : 1;     // Long-Mode Segment Limit Enable unsupported
    uint32_t tlb_flush_nested : 1;  // INVLPGB RAX[5] bit can be set
    uint32_t : 1;                   // Reserved
    uint32_t amd_ppin : 1;          // Protected Processor Inventory Number
    uint32_t amd_ssbd : 1;          // Speculative Store Bypass Disable
    uint32_t virt_ssbd : 1;         // virtualized SSBD (Speculative Store Bypass Disable)
    uint32_t amd_ssb_no : 1;        // SSBD is not needed (fixed in hardware)
    uint32_t cppc : 1;              // Collaborative Processor Performance Control
    uint32_t amd_psfd : 1;          // Predictive Store Forward Disable
    uint32_t btc_no : 1;            // CPU not affected by Branch Type Confusion
    uint32_t ibpb_ret : 1;          // IBPB clears RSB/RAS too
    uint32_t branch_sampling : 1;   // Branch Sampling
    // ecx
    uint32_t cpu_nthreads : 8;      // Number of physical threads - 1
    uint32_t : 4;                   // Reserved
    uint32_t apicid_coreid_len : 4; // Number of thread core ID bits (shift) in APIC ID
    uint32_t perf_tsc_len : 2;      // Performance time-stamp counter size
    uint32_t : 14;                  // Reserved
    // edx
    uint32_t invlpgb_max_pages : 16; // INVLPGB maximum page count
    uint32_t rdpru_max_reg_id : 16;  // RDPRU max register ID (ECX input)
} PACKED leaf_0x80000008_0_t;

/*
 * Leaf 0x8000000a
 * AMD SVM (Secure Virtual Machine)
 */

typedef struct leaf_0x8000000a_0
{
    // eax
    uint32_t svm_version : 8; // SVM revision number
    uint32_t : 24;            // Reserved
    // ebx
    uint32_t svm_nasid : 32; // Number of address space identifiers (ASID)
    // ecx
    uint32_t : 4;     // Reserved
    uint32_t pml : 1; // Page Modification Logging (PML)
    uint32_t : 27;    // Reserved
    // edx
    uint32_t nested_pt : 1;       // Nested paging
    uint32_t lbr_virt : 1;        // LBR virtualization
    uint32_t svm_lock : 1;        // SVM lock
    uint32_t nrip_save : 1;       // NRIP save support on #VMEXIT
    uint32_t tsc_rate_msr : 1;    // MSR-based TSC rate control
    uint32_t vmcb_clean : 1;      // VMCB clean bits support
    uint32_t flush_by_asid : 1;   // Flush by ASID + Extended VMCB TLB_Control
    uint32_t decode_assists : 1;  // Decode Assists support
    uint32_t : 2;                 // Reserved
    uint32_t pause_filter : 1;    // Pause intercept filter
    uint32_t : 1;                 // Reserved
    uint32_t pf_threshold : 1;    // Pause filter threshold
    uint32_t avic : 1;            // Advanced virtual interrupt controller
    uint32_t : 1;                 // Reserved
    uint32_t v_vmsave_vmload : 1; // Virtual VMSAVE/VMLOAD (nested virtualization)
    uint32_t v_gif : 1;           // Virtualize the Global Interrupt Flag
    uint32_t gmet : 1;            // Guest mode execution trap
    uint32_t x2avic : 1;          // Virtual x2APIC
    uint32_t sss_check : 1;       // Supervisor Shadow Stack restrictions
    uint32_t v_spec_ctrl : 1;     // Virtual SPEC_CTRL
    uint32_t ro_gpt : 1;          // Read-Only guest page table support
    uint32_t : 1;                 // Reserved
    uint32_t h_mce_override : 1;  // Host MCE override
    uint32_t tlbsync_int : 1;     // TLBSYNC intercept + INVLPGB/TLBSYNC in VMCB
    uint32_t nmi_virt : 1;        // NMI virtualization
    uint32_t ibs_virt : 1;        // IBS Virtualization
    uint32_t ext_lvt_off_chg : 1; // Extended LVT offset fault change
    uint32_t svme_addr_chk : 1;   // Guest SVME address check
    uint32_t : 3;                 // Reserved
} PACKED leaf_0x8000000a_0_t;

/*
 * Leaf 0x80000019
 * AMD TLB characteristics for 1GB pages
 */

typedef struct leaf_0x80000019_0
{
    // eax
    uint32_t l1_itlb_1g_nentries : 12; // L1 iTLB #entries, 1G pages
    uint32_t l1_itlb_1g_assoc : 4;     // L1 iTLB associativity, 1G pages
    uint32_t l1_dtlb_1g_nentries : 12; // L1 dTLB #entries, 1G pages
    uint32_t l1_dtlb_1g_assoc : 4;     // L1 dTLB associativity, 1G pages
    // ebx
    uint32_t l2_itlb_1g_nentries : 12; // L2 iTLB #entries, 1G pages
    uint32_t l2_itlb_1g_assoc : 4;     // L2 iTLB associativity, 1G pages
    uint32_t l2_dtlb_1g_nentries : 12; // L2 dTLB #entries, 1G pages
    uint32_t l2_dtlb_1g_assoc : 4;     // L2 dTLB associativity, 1G pages
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x80000019_0_t;

/*
 * Leaf 0x8000001a
 * AMD instruction optimizations
 */

typedef struct leaf_0x8000001a_0
{
    // eax
    uint32_t fp_128 : 1;         // Internal FP/SIMD exec data path is 128-bits wide
    uint32_t movu_preferred : 1; // SSE: MOVU* better than MOVL*/MOVH*
    uint32_t fp_256 : 1;         // Internal FP/SSE exec data path is 256-bits wide
    uint32_t : 29;               // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x8000001a_0_t;

/*
 * Leaf 0x8000001b
 * AMD IBS (Instruction-Based Sampling)
 */

typedef struct leaf_0x8000001b_0
{
    // eax
    uint32_t ibs_flags : 1;           // IBS feature flags
    uint32_t ibs_fetch_sampling : 1;  // IBS fetch sampling
    uint32_t ibs_op_sampling : 1;     // IBS execution sampling
    uint32_t ibs_rdwr_op_counter : 1; // IBS read/write of op counter
    uint32_t ibs_op_count : 1;        // IBS OP counting mode
    uint32_t ibs_branch_target : 1;   // IBS branch target address reporting
    uint32_t ibs_op_counters_ext : 1; // IBS IbsOpCurCnt/IbsOpMaxCnt extend by 7 bits
    uint32_t ibs_rip_invalid_chk : 1; // IBS invalid RIP indication
    uint32_t ibs_op_branch_fuse : 1;  // IBS fused branch micro-op indication
    uint32_t ibs_fetch_ctl_ext : 1;   // IBS Fetch Control Extended MSR
    uint32_t ibs_op_data_4 : 1;       // IBS op data 4 MSR
    uint32_t ibs_l3_miss_filter : 1;  // IBS L3-miss filtering (Zen4+)
    uint32_t : 20;                    // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x8000001b_0_t;

/*
 * Leaf 0x8000001c
 * AMD LWP (Lightweight Profiling)
 */

typedef struct leaf_0x8000001c_0
{
    // eax
    uint32_t os_lwp_avail : 1; // OS: LWP is available to application programs
    uint32_t os_lwpval : 1;    // OS: LWPVAL instruction
    uint32_t os_lwp_ire : 1;   // OS: Instructions Retired Event
    uint32_t os_lwp_bre : 1;   // OS: Branch Retired Event
    uint32_t os_lwp_dme : 1;   // OS: Dcache Miss Event
    uint32_t os_lwp_cnh : 1;   // OS: CPU Clocks Not Halted event
    uint32_t os_lwp_rnh : 1;   // OS: CPU Reference clocks Not Halted event
    uint32_t : 22;             // Reserved
    uint32_t os_lwp_cont : 1;  // OS: LWP sampling in continuous mode
    uint32_t os_lwp_ptsc : 1;  // OS: Performance Time Stamp Counter in event records
    uint32_t os_lwp_int : 1;   // OS: Interrupt on threshold overflow
    // ebx
    uint32_t lwp_lwpcb_sz : 8;     // Control Block size, in quadwords
    uint32_t lwp_event_sz : 8;     // Event record size, in bytes
    uint32_t lwp_max_events : 8;   // Max EventID supported
    uint32_t lwp_event_offset : 8; // Control Block events area offset
    // ecx
    uint32_t lwp_latency_max : 5;    // Cache latency counters number of bits
    uint32_t lwp_data_addr : 1;      // Cache miss events report data cache address
    uint32_t lwp_latency_rnd : 3;    // Cache latency rounding amount
    uint32_t lwp_version : 7;        // LWP version
    uint32_t lwp_buf_min_sz : 8;     // LWP event ring buffer min size, 32 event record units
    uint32_t : 4;                    // Reserved
    uint32_t lwp_branch_predict : 1; // Branches Retired events can be filtered
    uint32_t lwp_ip_filtering : 1;   // IP filtering (IPI, IPF, BaseIP, and LimitIP @ LWPCP)
    uint32_t lwp_cache_levels : 1;   // Cache-related events: filter by cache level
    uint32_t lwp_cache_latency : 1;  // Cache-related events: filter by latency
    // edx
    uint32_t hw_lwp_avail : 1; // HW: LWP available
    uint32_t hw_lwpval : 1;    // HW: LWPVAL available
    uint32_t hw_lwp_ire : 1;   // HW: Instructions Retired Event
    uint32_t hw_lwp_bre : 1;   // HW: Branch Retired Event
    uint32_t hw_lwp_dme : 1;   // HW: Dcache Miss Event
    uint32_t hw_lwp_cnh : 1;   // HW: Clocks Not Halted event
    uint32_t hw_lwp_rnh : 1;   // HW: Reference clocks Not Halted event
    uint32_t : 22;             // Reserved
    uint32_t hw_lwp_cont : 1;  // HW: LWP sampling in continuous mode
    uint32_t hw_lwp_ptsc : 1;  // HW: Performance Time Stamp Counter in event records
    uint32_t hw_lwp_int : 1;   // HW: Interrupt on threshold overflow
} PACKED leaf_0x8000001c_0_t;

/*
 * Leaf 0x8000001d
 * AMD deterministic cache parameters
 */

typedef struct leaf_0x8000001d_n
{
    // eax
    uint32_t cache_type : 5;           // Cache type field
    uint32_t cache_level : 3;          // Cache level (1-based)
    uint32_t cache_self_init : 1;      // Self-initializing cache level
    uint32_t fully_associative : 1;    // Fully-associative cache
    uint32_t : 4;                      // Reserved
    uint32_t num_threads_sharing : 12; // Number of logical CPUs sharing cache
    uint32_t : 6;                      // Reserved
    // ebx
    uint32_t cache_linesize : 12;    // System coherency line size (0-based)
    uint32_t cache_npartitions : 10; // Physical line partitions (0-based)
    uint32_t cache_nways : 10;       // Ways of associativity (0-based)
    // ecx
    uint32_t cache_nsets : 31; // Cache number of sets (0-based)
    uint32_t : 1;              // Reserved
    // edx
    uint32_t wbinvd_rll_no_guarantee : 1; // WBINVD/INVD not guaranteed for Remote Lower-Level caches
    uint32_t ll_inclusive : 1;            // Cache is inclusive of Lower-Level caches
    uint32_t : 30;                        // Reserved
} PACKED leaf_0x8000001d_n_t;

#define LEAF_0x8000001d_SUBLEAF_N_FIRST 0
#define LEAF_0x8000001d_SUBLEAF_N_LAST  31

/*
 * Leaf 0x8000001e
 * AMD CPU topology
 */

typedef struct leaf_0x8000001e_0
{
    // eax
    uint32_t ext_apic_id : 32; // Extended APIC ID
    // ebx
    uint32_t core_id : 8;       // Unique per-socket logical core unit ID
    uint32_t core_nthreads : 8; // #Threads per core (zero-based)
    uint32_t : 16;              // Reserved
    // ecx
    uint32_t node_id : 8;           // Node (die) ID of invoking logical CPU
    uint32_t nnodes_per_socket : 3; // #nodes in invoking logical CPU's package/socket
    uint32_t : 21;                  // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x8000001e_0_t;

/*
 * Leaf 0x8000001f
 * AMD encrypted memory capabilities (SME/SEV)
 */

typedef struct leaf_0x8000001f_0
{
    // eax
    uint32_t sme : 1;                  // Secure Memory Encryption
    uint32_t sev : 1;                  // Secure Encrypted Virtualization
    uint32_t vm_page_flush : 1;        // VM Page Flush MSR
    uint32_t sev_encrypted_state : 1;  // SEV Encrypted State
    uint32_t sev_nested_paging : 1;    // SEV secure nested paging
    uint32_t vm_permission_levels : 1; // VMPL
    uint32_t rpmquery : 1;             // RPMQUERY instruction
    uint32_t vmpl_sss : 1;             // VMPL supervisor shadow stack
    uint32_t secure_tsc : 1;           // Secure TSC
    uint32_t virt_tsc_aux : 1;         // Hardware virtualizes TSC_AUX
    uint32_t sme_coherent : 1;         // Cache coherency enforcement across encryption domains
    uint32_t req_64bit_hypervisor : 1; // SEV guest mandates 64-bit hypervisor
    uint32_t restricted_injection : 1; // Restricted Injection supported
    uint32_t alternate_injection : 1;  // Alternate Injection supported
    uint32_t debug_swap : 1;           // SEV-ES: Full debug state swap
    uint32_t disallow_host_ibs : 1;    // SEV-ES: Disallowing IBS use by the host
    uint32_t virt_transparent_enc : 1; // Virtual Transparent Encryption
    uint32_t vmgexit_parameter : 1;    // SEV_FEATURES: VmgexitParameter
    uint32_t virt_tom_msr : 1;         // Virtual TOM MSR
    uint32_t virt_ibs : 1;             // SEV-ES guests: IBS state virtualization
    uint32_t : 4;                      // Reserved
    uint32_t vmsa_reg_protection : 1;  // VMSA register protection
    uint32_t smt_protection : 1;       // SMT protection
    uint32_t : 2;                      // Reserved
    uint32_t svsm_page_msr : 1;        // SVSM communication page MSR
    uint32_t nested_virt_snp_msr : 1;  // VIRT_RMPUPDATE/VIRT_PSMASH MSRs
    uint32_t : 2;                      // Reserved
    // ebx
    uint32_t pte_cbit_pos : 6;              // PTE bit number to enable memory encryption
    uint32_t phys_addr_reduction_nbits : 6; // Reduction of phys address space in bits
    uint32_t vmpl_count : 4;                // Number of VM permission levels (VMPL)
    uint32_t : 16;                          // Reserved
    // ecx
    uint32_t enc_guests_max : 32; // Max number of simultaneous encrypted guests
    // edx
    uint32_t min_sev_asid_no_sev_es : 32; // Minimum ASID for SEV-enabled SEV-ES-disabled guest
} PACKED leaf_0x8000001f_0_t;

/*
 * Leaf 0x80000020
 * AMD PQoS (Platform QoS) extended features
 */

typedef struct leaf_0x80000020_0
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t : 1;        // Reserved
    uint32_t mba : 1;    // Memory Bandwidth Allocation support
    uint32_t smba : 1;   // Slow Memory Bandwidth Allocation support
    uint32_t bmec : 1;   // Bandwidth Monitoring Event Configuration support
    uint32_t l3rr : 1;   // L3 Range Reservation support
    uint32_t abmc : 1;   // Assignable Bandwidth Monitoring Counters
    uint32_t sdciae : 1; // Smart Data Cache Injection (SDCI) Allocation Enforcement
    uint32_t : 25;       // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x80000020_0_t;

typedef struct leaf_0x80000020_1
{
    // eax
    uint32_t mba_limit_len : 32; // MBA enforcement limit size
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t mba_cos_max : 32; // MBA max Class of Service number (zero-based)
} PACKED leaf_0x80000020_1_t;

typedef struct leaf_0x80000020_2
{
    // eax
    uint32_t smba_limit_len : 32; // SMBA enforcement limit size
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t smba_cos_max : 32; // SMBA max Class of Service number (zero-based)
} PACKED leaf_0x80000020_2_t;

typedef struct leaf_0x80000020_3
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t bmec_num_events : 8; // BMEC number of bandwidth events available
    uint32_t : 24;                // Reserved
    // ecx
    uint32_t bmec_local_reads : 1;        // Local NUMA reads can be tracked
    uint32_t bmec_remote_reads : 1;       // Remote NUMA reads can be tracked
    uint32_t bmec_local_nontemp_wr : 1;   // Local NUMA non-temporal writes can be tracked
    uint32_t bmec_remote_nontemp_wr : 1;  // Remote NUMA non-temporal writes can be tracked
    uint32_t bmec_local_slow_mem_rd : 1;  // Local NUMA slow-memory reads can be tracked
    uint32_t bmec_remote_slow_mem_rd : 1; // Remote NUMA slow-memory reads can be tracked
    uint32_t bmec_all_dirty_victims : 1;  // Dirty QoS victims to all types of memory can be tracked
    uint32_t : 25;                        // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x80000020_3_t;

/*
 * Leaf 0x80000021
 * AMD extended CPU features 2
 */

typedef struct leaf_0x80000021_0
{
    // eax
    uint32_t no_nested_data_bp : 1;    // No nested data breakpoints
    uint32_t fsgs_non_serializing : 1; // WRMSR to {FS,GS,KERNEL_GS}_BASE is non-serializing
    uint32_t lfence_serializing : 1;   // LFENCE always serializing / synchronizes RDTSC
    uint32_t smm_page_cfg_lock : 1;    // SMM paging configuration lock
    uint32_t : 2;                      // Reserved
    uint32_t null_sel_clr_base : 1;    // Null selector clears base
    uint32_t upper_addr_ignore : 1;    // EFER MSR Upper Address Ignore
    uint32_t auto_ibrs : 1;            // EFER MSR Automatic IBRS
    uint32_t no_smm_ctl_msr : 1;       // SMM_CTL MSR not available
    uint32_t fsrs : 1;                 // Fast Short REP STOSB
    uint32_t fsrc : 1;                 // Fast Short REP CMPSB
    uint32_t : 1;                      // Reserved
    uint32_t prefetch_ctl_msr : 1;     // Prefetch control MSR
    uint32_t : 2;                      // Reserved
    uint32_t opcode_reclaim : 1;       // Reserves opcode space
    uint32_t user_cpuid_disable : 1;   // #GP when executing CPUID at CPL > 0
    uint32_t epsf : 1;                 // Enhanced Predictive Store Forwarding
    uint32_t : 3;                      // Reserved
    uint32_t wl_feedback : 1;          // Workload-based heuristic feedback to OS
    uint32_t : 1;                      // Reserved
    uint32_t eraps : 1;                // Enhanced Return Address Predictor Security
    uint32_t : 2;                      // Reserved
    uint32_t sbpb : 1;                 // Selective Branch Predictor Barrier
    uint32_t ibpb_brtype : 1;          // Branch predictions flushed from CPU branch predictor
    uint32_t srso_no : 1;              // No SRSO vulnerability
    uint32_t srso_uk_no : 1;           // No SRSO at user-kernel boundary
    uint32_t srso_msr_fix : 1;         // MSR BP_CFG[BpSpecReduce] SRSO mitigation
    // ebx
    uint32_t microcode_patch_size : 16; // Microcode patch size, in 16-byte units
    uint32_t rap_size : 8;              // Return Address Predictor size
    uint32_t : 8;                       // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x80000021_0_t;

/*
 * Leaf 0x80000022
 * AMD extended performance monitoring
 */

typedef struct leaf_0x80000022_0
{
    // eax
    uint32_t perfmon_v2 : 1;     // Performance monitoring v2
    uint32_t lbr_v2 : 1;         // Last Branch Record v2 extensions (LBR Stack)
    uint32_t lbr_pmc_freeze : 1; // Freezing core performance counters / LBR Stack
    uint32_t : 29;               // Reserved
    // ebx
    uint32_t n_pmc_core : 4;        // Number of core performance counters
    uint32_t lbr_v2_stack_size : 6; // Number of LBR stack entries
    uint32_t n_pmc_northbridge : 6; // Number of northbridge performance counters
    uint32_t n_pmc_umc : 6;         // Number of UMC performance counters
    uint32_t : 10;                  // Reserved
    // ecx
    uint32_t active_umc_bitmask : 32; // Active UMCs bitmask
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x80000022_0_t;

/*
 * Leaf 0x80000023
 * AMD multi-key encrypted memory
 */

typedef struct leaf_0x80000023_0
{
    // eax
    uint32_t mem_hmk_mode : 1; // MEM-HMK encryption mode
    uint32_t : 31;             // Reserved
    // ebx
    uint32_t mem_hmk_avail_keys : 16; // Total number of available encryption keys
    uint32_t : 16;                    // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x80000023_0_t;

/*
 * Leaf 0x80000026
 * AMD extended CPU topology
 */

typedef struct leaf_0x80000026_n
{
    // eax
    uint32_t x2apic_id_shift : 5;         // Bit width of this level (previous levels inclusive)
    uint32_t : 24;                        // Reserved
    uint32_t core_has_pwreff_ranking : 1; // This core has a power efficiency ranking
    uint32_t domain_has_hybrid_cores : 1; // This domain level has hybrid (E, P) cores
    uint32_t domain_core_count_asymm : 1; // The 'Core' domain has asymmetric cores count
    // ebx
    uint32_t domain_lcpus_count : 16;  // Number of logical CPUs at this domain instance
    uint32_t core_pwreff_ranking : 8;  // This core's static power efficiency ranking
    uint32_t core_native_model_id : 4; // This core's native model ID
    uint32_t core_type : 4;            // This core's type
    // ecx
    uint32_t domain_level : 8; // This domain level (subleaf ID)
    uint32_t domain_type : 8;  // This domain type
    uint32_t : 16;             // Reserved
    // edx
    uint32_t x2apic_id : 32; // x2APIC ID of current logical CPU
} PACKED leaf_0x80000026_n_t;

#define LEAF_0x80000026_SUBLEAF_N_FIRST 0
#define LEAF_0x80000026_SUBLEAF_N_LAST  3

/*
 * Leaf 0x80860000
 * Maximum Transmeta leaf + CPU vendor string
 */

typedef struct leaf_0x80860000_0
{
    // eax
    uint32_t max_tra_leaf : 32; // Maximum Transmeta leaf
    // ebx
    uint32_t cpu_vendorid_0 : 32; // Transmeta vendor ID string bytes 0 - 3
    // ecx
    uint32_t cpu_vendorid_2 : 32; // Transmeta vendor ID string bytes 8 - 11
    // edx
    uint32_t cpu_vendorid_1 : 32; // Transmeta vendor ID string bytes 4 - 7
} PACKED leaf_0x80860000_0_t;

/*
 * Leaf 0x80860001
 * Transmeta extended CPU features
 */

typedef struct leaf_0x80860001_0
{
    // eax
    uint32_t stepping : 4;       // Stepping ID
    uint32_t base_model : 4;     // Base CPU model ID
    uint32_t base_family_id : 4; // Base CPU family ID
    uint32_t cpu_type : 2;       // CPU type
    uint32_t : 18;               // Reserved
    // ebx
    uint32_t cpu_rev_mask_minor : 8; // CPU revision ID, mask minor
    uint32_t cpu_rev_mask_major : 8; // CPU revision ID, mask major
    uint32_t cpu_rev_minor : 8;      // CPU revision ID, minor
    uint32_t cpu_rev_major : 8;      // CPU revision ID, major
    // ecx
    uint32_t cpu_base_mhz : 32; // CPU nominal frequency, in MHz
    // edx
    uint32_t recovery : 1; // Recovery CMS is active (after bad flush)
    uint32_t longrun : 1;  // LongRun power management capabilities
    uint32_t : 1;          // Reserved
    uint32_t lrti : 1;     // LongRun Table Interface
    uint32_t : 28;         // Reserved
} PACKED leaf_0x80860001_0_t;

/*
 * Leaf 0x80860002
 * Transmeta CMS (Code Morphing Software)
 */

typedef struct leaf_0x80860002_0
{
    // eax
    uint32_t cpu_rev_id : 32; // CPU revision ID
    // ebx
    uint32_t cms_rev_mask_2 : 8; // CMS revision ID, mask component 2
    uint32_t cms_rev_mask_1 : 8; // CMS revision ID, mask component 1
    uint32_t cms_rev_minor : 8;  // CMS revision ID, minor
    uint32_t cms_rev_major : 8;  // CMS revision ID, major
    // ecx
    uint32_t cms_rev_mask_3 : 32; // CMS revision ID, mask component 3
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0x80860002_0_t;

/*
 * Leaf 0x80860003
 * Transmeta CPU information string, bytes 0 - 15
 */

typedef struct leaf_0x80860003_0
{
    // eax
    uint32_t cpu_info_0 : 32; // CPU info string bytes 0 - 3
    // ebx
    uint32_t cpu_info_1 : 32; // CPU info string bytes 4 - 7
    // ecx
    uint32_t cpu_info_2 : 32; // CPU info string bytes 8 - 11
    // edx
    uint32_t cpu_info_3 : 32; // CPU info string bytes 12 - 15
} PACKED leaf_0x80860003_0_t;

/*
 * Leaf 0x80860004
 * Transmeta CPU information string, bytes 16 - 31
 */

typedef struct leaf_0x80860004_0
{
    // eax
    uint32_t cpu_info_4 : 32; // CPU info string bytes 16 - 19
    // ebx
    uint32_t cpu_info_5 : 32; // CPU info string bytes 20 - 23
    // ecx
    uint32_t cpu_info_6 : 32; // CPU info string bytes 24 - 27
    // edx
    uint32_t cpu_info_7 : 32; // CPU info string bytes 28 - 31
} PACKED leaf_0x80860004_0_t;

/*
 * Leaf 0x80860005
 * Transmeta CPU information string, bytes 32 - 47
 */

typedef struct leaf_0x80860005_0
{
    // eax
    uint32_t cpu_info_8 : 32; // CPU info string bytes 32 - 35
    // ebx
    uint32_t cpu_info_9 : 32; // CPU info string bytes 36 - 39
    // ecx
    uint32_t cpu_info_10 : 32; // CPU info string bytes 40 - 43
    // edx
    uint32_t cpu_info_11 : 32; // CPU info string bytes 44 - 47
} PACKED leaf_0x80860005_0_t;

/*
 * Leaf 0x80860006
 * Transmeta CPU information string, bytes 48 - 63
 */

typedef struct leaf_0x80860006_0
{
    // eax
    uint32_t cpu_info_12 : 32; // CPU info string bytes 48 - 51
    // ebx
    uint32_t cpu_info_13 : 32; // CPU info string bytes 52 - 55
    // ecx
    uint32_t cpu_info_14 : 32; // CPU info string bytes 56 - 59
    // edx
    uint32_t cpu_info_15 : 32; // CPU info string bytes 60 - 63
} PACKED leaf_0x80860006_0_t;

/*
 * Leaf 0x80860007
 * Transmeta live CPU information
 */

typedef struct leaf_0x80860007_0
{
    // eax
    uint32_t cpu_cur_mhz : 32; // Current CPU frequency, in MHz
    // ebx
    uint32_t cpu_cur_voltage : 32; // Current CPU voltage, in millivolts
    // ecx
    uint32_t cpu_cur_perf_pctg : 32; // Current CPU performance percentage, 0 - 100
    // edx
    uint32_t cpu_cur_gate_delay : 32; // Current CPU gate delay, in femtoseconds
} PACKED leaf_0x80860007_0_t;

/*
 * Leaf 0xc0000000
 * Maximum Centaur/Zhaoxin leaf
 */

typedef struct leaf_0xc0000000_0
{
    // eax
    uint32_t max_cntr_leaf : 32; // Maximum Centaur/Zhaoxin leaf
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t : 32; // Reserved
} PACKED leaf_0xc0000000_0_t;

/*
 * Leaf 0xc0000001
 * Centaur/Zhaoxin extended CPU features
 */

typedef struct leaf_0xc0000001_0
{
    // eax
    uint32_t : 32; // Reserved
    // ebx
    uint32_t : 32; // Reserved
    // ecx
    uint32_t : 32; // Reserved
    // edx
    uint32_t ccs_sm2 : 1;        // CCS SM2 instructions
    uint32_t ccs_sm2_en : 1;     // CCS SM2 enabled
    uint32_t rng : 1;            // Random Number Generator
    uint32_t rng_en : 1;         // RNG enabled
    uint32_t ccs_sm3_sm4 : 1;    // CCS SM3 and SM4 instructions
    uint32_t ccs_sm3_sm4_en : 1; // CCS SM3/SM4 enabled
    uint32_t ace : 1;            // Advanced Cryptography Engine
    uint32_t ace_en : 1;         // ACE enabled
    uint32_t ace2 : 1;           // Advanced Cryptography Engine v2
    uint32_t ace2_en : 1;        // ACE v2 enabled
    uint32_t phe : 1;            // PadLock Hash Engine
    uint32_t phe_en : 1;         // PHE enabled
    uint32_t pmm : 1;            // PadLock Montgomery Multiplier
    uint32_t pmm_en : 1;         // PMM enabled
    uint32_t : 2;                // Reserved
    uint32_t parallax : 1;       // Parallax auto adjust processor voltage
    uint32_t parallax_en : 1;    // Parallax enabled
    uint32_t : 2;                // Reserved
    uint32_t tm3 : 1;            // Thermal Monitor v3
    uint32_t tm3_en : 1;         // TM v3 enabled
    uint32_t : 3;                // Reserved
    uint32_t phe2 : 1;           // PadLock Hash Engine v2 (SHA384/SHA512)
    uint32_t phe2_en : 1;        // PHE v2 enabled
    uint32_t rsa : 1;            // RSA instructions (XMODEXP/MONTMUL2)
    uint32_t rsa_en : 1;         // RSA instructions enabled
    uint32_t : 3;                // Reserved
} PACKED leaf_0xc0000001_0_t;

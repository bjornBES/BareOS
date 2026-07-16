/*
 * File: setup.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/asm/drivers/I8042/I8042.h"
#include "kernel/asm/segment/gdt.h"
#include "kernel/asm/acpi/hpet/hpet.h"
#include "kernel/asm/timer/pit.h"
#include "kernel/asm/MSR/MSR.h"
#include "kernel/irq.h"
#include "kernel/ivt.h"
#include "kernel/syscall.h"
#include "kernel/mmu.h"
#include "kernel/acpi/acpi.h"
#include "kernel/acpi/fadt.h"
#include "kernel.h"

#include "acpi/table.h"
#include "mm/mmu/mmu.h"

#include "task/tss.h"
#include "exceptions/isr.h"
#include "exceptions/exception.h"

#include "time/timer.h"

#include "debug/debug.h"
#include <boot/bootparams.h>
#include "smp/smp.h"

#define MODULE "X86"

extern void kernel_breakpoint(intr_frame_t *regs);

int breakpoint(intr_frame_t *regs)
{
    char buf[2048];
    log_debug("breakpoint", "breakpoint\n");
    log_debug("breakpoint", "%s", buf);

    // kernel_breakpoint(regs);

    // do something here
    return RETURN_GOOD;
}

int write_registers(intr_frame_t *regs)
{
    char buf[2048];
    log_debug("debug", "debug");
    log_debug("debug", "%s", buf);
    // do something here
    return RETURN_GOOD;
}

int double_fault(intr_frame_t *regs)
{
    char buf[2048];
    log_err("double", "double fault");
    log_debug("double", "%s", buf);
    panic("double fault", __FILE__, __LINE__, "double fault");
    return RETURN_FAILED;
}

int general_protection_fault(intr_frame_t *regs)
{
    log_err("GPF", "General Protection Fault %x", regs->error);
    ivt_dump_frame(regs);
    x86_GDT_dump_selector(regs->error);
    panic("GPF", __FILE__, __LINE__, "KERNEL GOT a GPF from %u ss", regs->error);
    // R9 = 0x464c452e54494e = "FLE.TIN"
    // R10 = 0x492f6e69622f3a72 = "I/nib/:r"
    // R11 = 0x6573752f = "esu/"
    // 0x46_4c_45_2e_54_49_4e:49_2f_6e_69_62_2f_3a_72:65_73_75_2f
    //   F  L  E  .  T  I  N  I  /  n  i  b  /  :  r  e  s  u  /
    //   46 4c 45 2e 54 49 4e 49 2f 6e 69 62 2f 3a 72 65 73 75 2f
    // or = /user:/bin/INIT.ELF
    // smart shit GCC
    return RETURN_FAILED;
}

uint8_t PF_times = 0;

int page_fault(intr_frame_t *regs)
{
    PF_times++;
    if (PF_times == 1)
    {
        return RETURN_GOOD;
    }
    vaddr_t cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));

    fprintf(VFS_FD_DEBUG, "========== PAGE FAULT ==========\n");
    fprintf(VFS_FD_DEBUG, "\t{ regs @ %p, cr2 = %p }\n", regs, cr2);
    ivt_dump_frame(regs);
    fprintf(VFS_FD_DEBUG, "\t{ error = 0x%x }\n", regs->error);
    fprintf(VFS_FD_DEBUG, "========== PAGE FAULT ==========\n");

    panic("PF", __FILE__, __LINE__, "KERNEL GOT a PF at %p", cr2);
    return RETURN_FAILED;
}

boot_params_t *setup_arch(boot_params_t *bootParams)
{
    x86_GDT_initialize();
    log_debug(MODULE, "GDT is done");

    x86_GDT_load(&gdt_descriptor, gdt_table);
    log_debug(MODULE, "GDT is loaded");

    tss_load(TSS_SELECTOR);

    exception_init();

    ivt_arch_set_handler(EXC_DB, write_registers);
    ivt_arch_set_handler(EXC_BP, breakpoint);
    ivt_arch_set_handler(EXC_DF, double_fault);
    ivt_arch_set_handler(EXC_GP, general_protection_fault);
    ivt_arch_set_handler(EXC_PF, page_fault);

    ivt_arch_init();
    log_debug(MODULE, "IDT is done");

    arch_syscall_init();

    mmu_arch_init(bootParams);

    irq_arch_enable();

    boot_params_t *bp;
    {
        log_info("BOOT", "====Moving boot params====");
        bp = malloc(sizeof(boot_params_t));
        vaddr_t virt_bootParams = ((vaddr_t)bootParams + PAGE_SIZE);
        mmu_map_region(&kernel_page, PAGE_SIZE, 0, sizeof(boot_params_t), kernel_text_flags);
        log_debug("BOOT", "Mapped boot params");
        log_debug("BOOT", "Copying from %p to %p", bootParams, bp);
        memcpy(bp, (void *)virt_bootParams, sizeof(boot_params_t));
        log_debug("BOOT", "Freeing boot params");
        mmu_free_region(&kernel_page, virt_bootParams, sizeof(boot_params_t));
        log_debug(MODULE, "bootParams @ %p", bp);
    }

    acpi_arch_init(bp);

    irq_arch_initialize();
    log_debug(MODULE, "IRQ init");

    pit_init();

    hpet_init();
    fadt_init();

    I8042_init();

    log_debug(MODULE, "here");

    irq_arch_enable();
    return bp;
}

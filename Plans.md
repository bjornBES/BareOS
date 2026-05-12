# Bare OS

Bare OS is a DOS/Unix style OS that will support old intel CPU's like the 8080 and 8088 but will also have support for other features for other CPU's like the i386 and x86.

## Phase 0.5 Bootloader

The bootloader is tasked to support all x86 CPU's all the way down to the 8086/8088 by using CPUID and other ways to make it work. There will be 2 kernels the bootloader can load

1. is the [normal kernel](__#32-bit-p-mode-kernel__) that needs 32 bit p mode and will be a Higher Half Kernel.
2. is the [16 bit kernel](__#16-bit-kernel__) that only needs 16 bits to work and will be loaded at the __end symbol in the bootloader.

### 16 bit Kernel

with only 16 bits the kernel needs to use the bios for most of it's functions, and so the first thing that is needed to be done, is to make a BIOS device.

#### Phase 1 simple DOS

- [ ] BIOS device management (hook functions)

## 32/64 bit Kernel

### Phase 1 simple DOS and Unix

- [X] GDT & IDT + driver for 8259 PIC
- [X] E9 driver + debug
- [X] 32 bit Paging
- [X] PCI
- [X] Small VFS
- [X] AHCI (read-only)
- [X] Simple Unix style device management
- [X] FAT32 (read-only)
- [X] Print Text using font
- [X] Disk Partitions
- [X] Serial Driver
  - [X] UART
- [X] VFS (read-only)

### Phase 2 Advanced device management

- [X] Keyboard API
  - [X] Make a keyboard API
  - [X] PS/2 driver
- [/] Simple kernel/debug shell
- [X] TSS + ring 3
- [X] System calls (int 0x80)
- [X] file loader (e.g. ELF)
  - [X] jump to user space
- [X] AHCI with write support
- [X] FAT32 with write support
- [X] VFS with write support

### Phase 3 64 bits

this is optional if the CPUID EAX=0x80000001 bit 29 in EDX is 0
do it a kernel loader
This is what the kernel loader should do in order right after the bootloader is done.

- [X] Add support for Physical Address Extension
- [X] cli and set new segments + ss
- [X] set stack to kernel loader stack
- [X] set PAE bit in CR4
- [X] Load CR3 with phys address of PML4
- [X] Enable long mode using magic guess
  - mov ecx, 0xC0000080
  - rdmsr
  - or eax, (1 << 8)
  - wrmsr
- [X] Enable paging again
- [X] Rewrite the kernel and project again.

### Phase 4 Processes in full

- [X] Process management
  - [X] Process states
  - [X] Process termination
  - [X] Process creation
    - [X] Allocate memory for process
    - [X] Load executable into memory using file loader
    - [X] Allocate page tables for process
    - [X] Set up user space stack
    - [X] Set up user space heap
    - [X] Jump to user space using iret/iretq
      - [X] Switch to user space Page tables
- [X] Virtual memory management
- [X] malloc/free in userspace

#### Phase 5 SMP, Threads and Scheduler

- [X] Process Control Block
  - [X] Full PCB struct (registers, state, priority, timeslice)
  - [X] Save/restore CPU context on switch (rsp, rbp, callee-saved regs)
  - [X] Kernel stack per process

- [X] Scheduler
  - [X] Process queue
    - [X] Ready queue
    - [X] Sleep queue
  - [X] Round-robin scheduler
  - [X] Timer IRQ drives preemption
    - [X] PIT or APIC timer
      - [X] PIT
      - [X] APIC
  - [X] Voluntary yield() syscall
  - [X] sleep() / wakeup()
    - [X] Move between queues via PROC_STATE_SLEEP
    - [X] sleep()
    - [X] wakeup()
  - [X] Idle process (pid 0, runs when ready queue is empty)

- [X] Threads
  - [X] Thread struct (shares vma_memory_t with parent process)
  - [X] Thread creation / exit
  - [X] Per-thread kernel stack
  - [X] Thread scheduler integration

- [X] SMP
  - [X] ACPI/MADT parse — enumerate APs
    - [X] Get the ACPI/MADT in a "working" state
    - [X] Finish the job.
  - [X] AP startup (SIPI sequence)
  - [X] Per-CPU data structure
    - [X] cpu_t — current process, kernel stack, GDT/TSS per core
  - [X] APIC timer per core drives per-core scheduler
  - [X] Spinlocks / basic SMP synchronisation

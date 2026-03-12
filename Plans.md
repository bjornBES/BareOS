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

### 32 bit p mode Kernel

#### Phase 1 simple DOS and Unix

- [X] GDT & IDT + driver for 8259 PIC
- [X] E9 driver + debug
- [X] 32 bit Paging
- [X] PCI
- [X] Small VFS
- [X] AHCI (read-only)
- [X] Simple Unix style device management
- [X] FAT32 (read-only)
- [/] Print Text using font
- [X] Disk Partitions
- [X] Serial Driver
  - [X] UART
- [X] VFS (read-only)

#### Phase 2 Advanced device management

- [ ] Keyboard (PS/2) driver
  - [ ] Keyboard API
- [ ] Simple kernel/debug shell
- [ ] TSS + ring 3
- [ ] System calls (int 0x80)
- [ ] file loader (e.g. ELF)
  - [ ] jump to user space
- [ ] AHCI with write support
- [ ] FAT32 with write support
- [ ] VFS with write support

#### Phase 3 64 bits

this is optional if the CPUID EAX=0x80000001 bit 29 in EDX is 0
do it a kernel loader
This is what the kernel loader should do in order right after the bootloader is done.

- [ ] Add support for Physical Address Extension
- [ ] cli and set new segments + ss
- [ ] set stack to kernel loader stack
- [ ] set PAE bit in CR4
- [ ] Load CR3 with phys address of PML4
- [ ] Enable long mode using magic guess
  - mov ecx, 0xC0000080
  - rdmsr
  - or eax, (1 << 8)
  - wrmsr
- [ ] Enable paging again

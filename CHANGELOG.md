# Changelog

## [0.1.0.0](https://github.com/BjornBEs/BareOS/tree/0.1.0.0) (2026-09-08)

### Bootloader

* Removed: leftover debug logs from the FAT and ELF drivers

### Kernel

Arch:

* Added: the ability to have architecture specific errno numbers
* Added: the ability for architectures to have runtime data in the bootparams

Arch x86:

* Changed: the stack_top linker symbol and added it to the [entry.asm](src/kernel/arch/x86/entry/entry.asm) file
* Added: support for paging64 and functions to map memory addresses
* Removed: checks for HANDLER_IN_USE in the ivt

mm:

* Added: pmm registering of allocators and memory allocation functions
* Added: the buddy allocator
* Added: support to use other pmm allocates
* Added: functions to map regions in the kernel side mmu
* Added: kmalloc functions

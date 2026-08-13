# Bare OS

Bare OS is a DOS/Unix style OS that will support old intel CPU's like the 8080 and 8088 but will also have support for other features for other CPU's like the i386 and x86.

## Phase 0.5 Bootloader

The bootloader is tasked to support all x86 CPU's all the way down to the 8086/8088 by using CPUID and other ways to make it work. There will be 2 kernels the bootloader can load

1. is the [normal kernel](__#32-bit-p-mode-kernel__) that needs 32 bit p mode and will be a Higher Half Kernel.
2. is the [16 bit kernel](__#16-bit-kernel__) that only needs 16 bits to work and will be loaded at the __end symbol in the bootloader.

### 16 bit Kernel

with only 16 bits the kernel needs to use the bios for most of it's functions, and so the first thing that is needed to be done, is to make a BIOS device.

#### Phase 1 simple DOS

## 32/64 bit Kernel

### Phase 1 simple DOS and Unix

### Phase 2 Advanced device management

### Phase 3 64 bits

### Phase 4 Processes in full

### Phase 5 SMP, Threads and Scheduler

### Phase 6 Make shit better

## 64/32 TODO list

## Bootloader TODO list

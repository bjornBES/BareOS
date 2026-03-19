# Bare OS

BareOS is a small, low-level, Linux-inspired operating system designed to run close to the metal.

It targets old and constrained x86 hardware where DOS is too limited, but modern operating systems are too heavy. BareOS aims to be practical, understandable, and hackable — a system you can fully reason about from boot sector to userspace.

The name BareOS reflects its philosophy: minimal abstraction, explicit design, and full control over the machine.

## Building

Install the following dependencies:

``` bash
# Ubuntu, Debian:
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasm mtools wget python3 python3-pip python3-parted dosfstools libguestfs-tools qemu-system-x86

# Fedora:
sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo nasm mtools wget python3 python3-pip python3-pyparted dosfstools qemu-system-x86

# Arch & Arch-based:
paru -S gcc make bison flex libgmp-static libmpc mpfr texinfo nasm mtools qemu-system-x86 python3
```

Then you must run ```python3 -m pip install -r requirements.txt```

Run `make toolchain`, this should download and build the required tools (binutils and GCC). If you encounter errors during this step, you might have to modify `build_scripts/config.env` and try a different version of **binutils** and **gcc**. Using the same versions as in your distribution's repositories is your best bet. Run `make clean-toolchain-all` to delete all toolchain related files and start over.

Finally, you should be able to run `make`. Use `make run` to test your OS using qemu. If you need to debug then run `make debug`

## CEXE Binary Format Specification

### File Overview

| Section           | Offset | Size                    | Description                                                             |
| ----------------- | ------ | ----------------------- | ----------------------------------------------------------------------- |
| **Header**        | `0x00` | 32 bytes                | General file header with magic, version, file size, and physical offset |
| **Section Table** | `0x20` | 4 × 16 bytes = 64 bytes | Contains entries for `.text`, `.data`, `.rodata`, and `.bss`            |
| **Program Data**  | `0x60` | Variable                | Raw binary data of the compiled program                                 |

### Header Structure (buildHeader)

| Field Name    | Offset (from start of file) | Size (bytes) | Type          | Example Value | Description                                                          |
| ------------- | --------------------------- | ------------ | ------------- | ------------- | -------------------------------------------------------------------- |
| `magic`       | `0x00`                      | 4            | `bytes[4]`    | `b'CEXE'`     | File signature that identifies this as a custom executable format    |
| `version`     | `0x04`                      | 1            | `uint8`       | `0x01`        | File format version number                                           |
| `file_length` | `0x05`                      | 4            | `uint32` (LE) | `00 20 01 00` | Total file length in bytes (includes header, sections, and contents) |
| `phys_offset` | `0x09`                      | 4            | `uint32` (LE) | `00 00 10 00` | Physical load address or memory offset for the program               |
| `reserved`    | `0x0D`                      | 19           | `bytes[19]`   | `00 … 00`     | Unused padding bytes (reserved for future use)                       |

Total Header Size: 32 bytes (0x20)

### Section Entry Structure

Each entry describes a single segment in memory (e.g., text, data, rodata, bss).

| Field Name     | Offset (relative to section entry) | Size (bytes) | Type         | Example Value             | Description                                                              |
| -------------- | ---------------------------------- | ------------ | ------------ | ------------------------- | ------------------------------------------------------------------------ |
| `name`         | `0x00`                             | 2            | `bytes[2]`   | `b'te'`                   | Section identifier (e.g., "te"=text, "da"=data, "ro"=rodata, "bs"=bss)   |
| `present_flag` | `0x02`                             | 1            | `uint8`      | `0x01`                    | 1 if section has data, 0 if empty                                        |
| `offset`       | `0x03`                             | 8            | `int64` (LE) | `00 00 00 00 00 00 00 00` | Offset (relative to file start or memory base) where this section begins |
| `size`         | `0x0B`                             | 4            | `int32` (LE) | `00 20 00 00`             | Size of the section in bytes                                             |
| `padding`      | `0x0F`                             | 1            | `byte`       | `00`                      | Unused byte for alignment                                                |

Total Section Size: 16 bytes (0x10)

### Section Table Layout

| Section   | Offset (in file) | Bytes | Description                                        |
| --------- | ---------------- | ----- | -------------------------------------------------- |
| `.text`   | `0x20`           | 16    | Code section (executable instructions)             |
| `.data`   | `0x30`           | 16    | Initialized writable data                          |
| `.rodata` | `0x40`           | 16    | Read-only constants or strings                     |
| `.bss`    | `0x50`           | 16    | Uninitialized zeroed memory (allocated at runtime) |

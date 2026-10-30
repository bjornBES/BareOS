# Bare OS

BareOS is a small, low-level, POSIX-compliant, Unix and DOS-inspired operating system designed to run close to the metal.

It targets old and constrained x86 hardware where DOS is too limited, but modern operating systems are too heavy. BareOS aims to be practical, understandable, and hackable — a system you can fully reason about from boot sector to userspace.

The name BareOS reflects its philosophy: minimal abstraction, explicit design, and full control over the machine and *it is also a joke if you understand Danish*.

## Building

Install the following dependencies:

``` bash
# Ubuntu, Debian:
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasm mtools wget python3 python3-pip python3-parted dosfstools libguestfs-tools qemu-system-x86

# Fedora:
sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo nasm mtools wget python3 python3-pip python3-pyparted dosfstools qemu-system-x86 xz

# Arch & Arch-based:
paru -S gcc make bison flex libgmp-static libmpc mpfr texinfo nasm mtools qemu-system-x86 python3
```

Then you must run ```python3 -m pip install -r requirements.txt```

Run `make toolchain`, this should download and build the required tools (binutils and GCC). If you encounter errors during this step, you might have to modify `config/config.env` and try a different version of **binutils** and **gcc**. Using the same versions as in your distribution's repositories is your best bet. Run `make clean-toolchain-all` to delete all toolchain related files and start over.

Finally, you should be able to run `make`. Use `make run` to test your OS using qemu. If you need to debug then run `make debug`

## Use of AI in this project

AI is used in this project, but only for:

- Write documentation and document the code
- Research and find documentation
- Make a commit template
- Get an idea of a given concept or design
- Write supporting files in languages I don't know (bash, the Python build scripts, and the makefile config)

That means AI is **not** used to "vibe-code" this OS, instead, AI is used as a tool and not as a replacement. The AI used is Claude (currently Sonnet 5.5).

Examples of where AI has been used:

1. In [kerrno.h](src/kernel/include/kerrno.h), AI wrote the comments describing the errors.
2. [gen_config.sh](scripts/gen_config.sh) was written entirely by AI, because I can't write bash.
3. The [build system](scripts/python/) and the [makefile config](mk) are in languages I don't know, so AI helped write them.
4. AI was used to help write many of the files in [documentation](documentation).

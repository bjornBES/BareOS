include config/config.env
export $(shell cut -d= -f1 config/config.env | grep -v '^#')

export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)/$(arch)_$(config)
export TOOLCHAIN_DIR = $(abspath toolchain)

export TARGET = ${arch}-elf
binPath = $(TOOLCHAIN_DIR)/$(TARGET)/bin
export TOOLCHAIN_INCLUDE_DIR = $(TOOLCHAIN_DIR)/$(TARGET)/lib/gcc/$(TARGET)/$(GCC_VERSION)
export INCLUDE_DIR = $(TOOLCHAIN_INCLUDE_DIR)/include

BINUTILS_VERSION = 2.37
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION = 11.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz

export CC = gcc
export CXX = g++
export LD = g++
export ASM = nasm
export AR = ar

export TARGET_ASM = nasm
export TARGET_AR  = $(binPath)/$(TARGET)-$(AR)
export TARGET_CC  = $(binPath)/$(TARGET)-$(CC)
export TARGET_CXX = $(binPath)/$(TARGET)-$(CXX)
export TARGET_LD  = $(binPath)/$(TARGET)-$(LD)
export TARGET_LIBS = $(BUILD_DIR)/libcore.a

export TARGET32_ASM = nasm
export TARGET32_AR  = $(TOOLCHAIN_DIR)/i686-elf/bin/i686-elf-$(AR)
export TARGET32_CC  = $(TOOLCHAIN_DIR)/i686-elf/bin/i686-elf-$(CC)
export TARGET32_LD  = $(TOOLCHAIN_DIR)/i686-elf/bin/i686-elf-$(CC)

export SRCARCH = $(arch)
ifeq ($(arch),i686)
        SRCARCH := x86
endif
ifeq ($(arch),x86_64)
        SRCARCH := x86
endif
export SOURCE_ARCH = arch/$(SRCARCH)

# --- Flags ---
export CFLAGS = -Wall -Werror -trigraphs                    \
    -Wno-error=unused-variable -Wno-error=unused-function   \
    -Wno-error=unused-label -Wno-error=deprecated           \
    -Wno-error=trigraphs -Werror=int-to-pointer-cast        \
    -masm=intel                                             \
    -DMAX_PATH_SIZE=$(max_path_length)                      \
    -DPAGING=$(enable_paging)                               \
    -nostdlib -ffreestanding -MMD -MP
# DO NOT SWITCH IT FROM INTEL
# I WILL FUCKING FIND YOU, AND KILL YOU.
# - BjornBEs 24-03-2026 13:55

export ASMFLAGS = -D PAGING=$(enable_paging)
export LINKFLAGS = -static
export LIBS = -lgcc


ifeq ($(config), debug)
    CFLAGS += -DDEBUG=1 -O0
	LINKFLAGS += -DDEBUG=1
endif

export floppyOutput = $(BUILD_DIR)/image.img

export TARGET32_ASMFLAGS  = $(ASMFLAGS) -f elf32 -D$(arch)=1 -I.
export TARGET32_CFLAGS    = $(CFLAGS) -std=c99
export TARGET32_CXXFLAGS  = $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti
export TARGET32_LINKFLAGS = $(LINKFLAGS) -nostdlib
export TARGET32_LIBS      = $(LIBS)

export TARGET64_ASMFLAGS  = $(ASMFLAGS) -f elf64 -D$(arch)=1 -I.
export TARGET64_CFLAGS    = $(CFLAGS) -std=c99 -mcmodel=kernel
export TARGET64_CXXFLAGS  = $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti
export TARGET64_LINKFLAGS = $(LINKFLAGS) -nostdlib
export TARGET64_LIBS      = $(LIBS)

export PAGING_ENABLE = 1
export MAX_PATH_SIZE = 512
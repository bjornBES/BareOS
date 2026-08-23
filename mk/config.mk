include config/config.env
export $(shell sed -n 's/^\([A-Za-z_][A-Za-z0-9_]*\)[[:space:]]*=.*/\1/p' config/config.env)

export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)/$(arch)_$(config)
export TOOLCHAIN_DIR = $(abspath $(toolchain))
export SYSROOT_DIR = $(abspath rootfs/user)

triple-x86_64 = x86_64-elf
triple-i686 = i686-elf
# triple-aarch64 = aarch64-elf

TARGET_TRIPLE = $(triple-$(arch))

ifeq ($(TARGET_TRIPLE),)
$(error unknown arch '$(arch)' no entry in triple-<arch> table)
endif

BINUTILS_VERSION = 2.37
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION = 11.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz

export TARGET = $(TARGET_TRIPLE)
export TOOLCHAIN_BIN_PATH = $(TOOLCHAIN_DIR)/$(TARGET)/bin
export TOOLCHAIN_INCLUDE_DIR = $(TOOLCHAIN_DIR)/$(TARGET)/lib/gcc/$(TARGET)/$(GCC_VERSION)
export INCLUDE_DIR = $(TOOLCHAIN_INCLUDE_DIR)/include

export AS_PREFIX = "AS: "
export CC_PREFIX = "CC: "
export CX_PREFIX = "CX: "
export LD_PREFIX = "LD: "
export AR_PREFIX = "AR: "

srcarch-x86_64 = x86
srcarch-i686 = x86

export SRCARCH = $(srcarch-$(arch))
export SOURCE_ARCH = arch/$(SRCARCH)

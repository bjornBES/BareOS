include config/config.env
export $(shell cut -d= -f1 config/config.env | grep -v '^#')

export CFLAGS = -Wall -Werror -trigraphs -Wno-error=unused-variable				\
				-Werror=int-to-pointer-cast	-Werror -masm=intel					\
				-Wno-error=unused-function -Wno-error=unused-label				\
				-Wno-error=deprecated -Wno-error=trigraphs						\
				-DMAX_PATH_SIZE=$(max_path_length) -DPAGING=$(enable_paging) 	\
				-nostdlib -ffreestanding
# DO NOT SWITCH IT FROM INTEL
# I WILL FUCKING FIND YOU, AND KILL YOU.
# - BjornBEs 24-03-2026 13:55

export ASMFLAGS = -D PAGING=$(enable_paging)
export CBLFLAGS = -std=cobol2002 -Wall -Werror -fsign=ASCII -I ./ -I $(SOURCE_DIR)/src/libs -static
export LINKFLAGS = -static
export LIBS = -lgcc

export CC = gcc
export CXX = g++
export LD = g++
export ASM = nasm
export AR = ar
export CBL = cobc

ifeq ($(config), debug)
    CFLAGS += -DDEBUG=1 -O0
	LINKFLAGS += -DDEBUG=1
else
    
endif


export floppyOutput = $(BUILD_DIR)/image.img

export TARGET = ${arch}-elf

binPath = $(TOOLCHAIN_DIR)/$(TARGET)/bin
binPath32 = $(TOOLCHAIN_DIR)/i686-elf/bin


export TARGET32_ASMFLAGS = $(ASMFLAGS) -f elf -D${arch}=1 -I.
export TARGET32_CFLAGS = $(CFLAGS) -std=c99 #-O2
export TARGET32_CXXFLAGS = $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti #-O2
export TARGET32_LINKFLAGS = $(LINKFLAGS) -nostdlib
export TARGET32_LIBS = $(LIBS)

export TARGET64_ASMFLAGS = $(ASMFLAGS) -f elf64 -D${arch}=1 -I.
export TARGET64_CFLAGS = $(CFLAGS) -std=c99 -mcmodel=kernel #-O2
export TARGET64_CXXFLAGS = $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti #-O2
export TARGET64_LINKFLAGS = $(LINKFLAGS) -nostdlib
export TARGET64_LIBS = $(LIBS)

export TARGET64_ASM = nasm
export TARGET64_AR = ${binPath}/$(TARGET)-$(AR)
export TARGET64_CC = ${binPath}/$(TARGET)-$(CC)
export TARGET64_CXX = ${binPath}/$(TARGET)-$(CXX)
export TARGET64_LD = ${binPath}/$(TARGET)-$(LD)

export TARGET32_ASM = nasm
export TARGET32_AR = ${binPath32}/i686-elf-$(AR)
export TARGET32_CC = ${binPath32}/i686-elf-$(CC)
export TARGET32_CXX = ${binPath32}/i686-elf-$(CXX)
export TARGET32_LD = ${binPath32}/i686-elf-$(LD)

export TARGET_LIBS = $(BUILD_DIR)/libcore.a

export SRCARCH = $(arch)

ifeq ($(arch),i686)
        SRCARCH := x86
endif
ifeq ($(arch),x86_64)
        SRCARCH := x86
endif

export SOURCE_ARCH = arch/$(SRCARCH)
export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)/$(arch)_$(config)
export FILE_DIR = $(abspath files)
export ROOT_DIR = $(FILE_DIR)/root
export TOOLCHAIN_INCLUDE_DIR = $(TOOLCHAIN_DIR)/$(TARGET)/lib/gcc/$(TARGET)/$(GCC_VERSION)
export INCLUDE_DIR = $(TOOLCHAIN_INCLUDE_DIR)/include
export TOOLCHAIN_DIR = $(abspath toolchain)

BINUTILS_VERSION = 2.37
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION = 11.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz

export PAGING_ENABLE = 1
export MAX_PATH_SIZE = 512
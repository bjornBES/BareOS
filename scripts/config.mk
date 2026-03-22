include config/config.env
export $(shell cut -d= -f1 config/config.env | grep -v '^#')

export CFLAGS = -Wall -Werror -trigraphs -Wno-error=unused-variable				\
				-Werror=int-to-pointer-cast	-Werror							\
				-Wno-error=unused-function -Wno-error=unused-label				\
				-Wno-error=deprecated -Wno-error=trigraphs						\
				-DMAX_PATH_SIZE=$(max_path_length) -DPAGING=$(enable_paging) 	\
				-nostdlib -ffreestanding
# -I /usr/local/i686-elf/include
# the -Wno-error=unused-variable flag is temp
export ASMFLAGS = -D PAGING=$(enable_paging)
export CBLFLAGS = -std=cobol2002 -Wall -Werror -fsign=ASCII -I ./ -I $(SOURCE_DIR)/src/libs -static
export CC = gcc
export CXX = g++
export LD = gcc
export ASM = nasm
export AR = ar
export CBL = cobc
export LINKFLAGS = -static
export LIBS = src/libs

ifeq ($(config), debug)
    CFLAGS += -DDEBUG=1 -O0
	LINKFLAGS += -DDEBUG=1
else
    
endif


export floppyOutput = $(BUILD_DIR)/image.img

export TARGET = ${arch}-elf
binPath = $(TOOLCHAIN_DIR)/$(TARGET)/bin
export TARGET_ASM = nasm

export TARGET_ASMFLAGS = $(ASMFLAGS) -f elf -D__${arch}__=1 -I.
export TARGET_CFLAGS = $(CFLAGS) -std=c99 #-O2
export TARGET_CXXFLAGS = $(CFLAGS) -std=c++17 -fno-exceptions -fno-rtti #-O2
export TARGET_LINKFLAGS = $(LINKFLAGS) -nostdlib -D__${arch}__=1

export TARGET_AR = ${binPath}/$(TARGET)-ar
export TARGET_CC = ${binPath}/$(TARGET)-gcc
export TARGET_CXX = ${binPath}/$(TARGET)-g++
export TARGET_LD = ${binPath}/$(TARGET)-g++
export TARGET_LIBS = $(BUILD_DIR)/libcore.a

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
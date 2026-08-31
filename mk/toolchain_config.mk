
include mk/config.mk

export CC = gcc
export CXX = g++
export LD = g++
export OBJCOPY = objcopy
export AR = ar
export AS = nasm

ifeq ($(use_bios),1)

BOOT_TRIPLE := i686-elf

export BOOT_CC := $(TOOLCHAIN_DIR)/$(BOOT_TRIPLE)/bin/$(BOOT_TRIPLE)-$(CC)
export BOOT_LD := $(TOOLCHAIN_DIR)/$(BOOT_TRIPLE)/bin/$(BOOT_TRIPLE)-$(LD)
export BOOT_AR := $(TOOLCHAIN_DIR)/$(BOOT_TRIPLE)/bin/$(BOOT_TRIPLE)-$(AR)
export BOOT_AS := $(AS)

ifeq (,$(wildcard $(BOOT_CC)))
# $(error use_bios=1 but bootloader toolchain missing: $(BOOT_CC))
endif

endif

export TARGET_CC      := $(TOOLCHAIN_BIN_PATH)/$(TARGET)-$(CC)
export TARGET_CXX     := $(TOOLCHAIN_BIN_PATH)/$(TARGET)-$(CXX)
export TARGET_LD      := $(TOOLCHAIN_BIN_PATH)/$(TARGET)-$(LD)
export TARGET_OBJCOPY := $(TOOLCHAIN_BIN_PATH)/$(TARGET)-$(OBJCOPY)
export TARGET_AR      := $(TOOLCHAIN_BIN_PATH)/$(TARGET)-$(AR)
export TARGET_AS      := $(AS)

ifeq (,$(wildcard $(TARGET_CC)))
# $(error toolchain missing: $(TARGET_CC), build it or check toolchain= in config.env)
endif

ifeq ($(config),debug)
export OPT_FLAGS := -O0 -g -DDEBUG
else ifeq ($(config),release)
export OPT_FLAGS := -O2 -DNDEBUG
else
$(error unknown config '$(config)' - must be debug or release)
endif

export ASFLAGS_DEFINE := -D__i686__=0 -D__x86_64__=0 -D__$(arch)__=1 -D__$(SRCARCH)__=1
export CFLAGS_DEFINE := -D__$(arch)__=1 -D__$(SRCARCH)__=1 -Werror
export COMMON_INCLUDE := -I./ -I./include \
	-I./arch/$(SRCARCH) -I./arch/$(SRCARCH)/include \
	-I$(SOURCE_DIR)/src/libs -I$(SOURCE_DIR)/src/libs/include \
	-I$(SOURCE_DIR)/src/libs/arch/$(SRCARCH) -I$(SOURCE_DIR)/src/libs/arch/$(SRCARCH)/include \
	-I./arch/generic -I./arch/generic/include \
	-I$(INCLUDE_DIR)

export COMMON_LIBS = -lgcc
export ASFLAGS_COMMON := $(ASFLAGS_DEFINE)
export CFLAGS_COMMON := -Wall $(CFLAGS_DEFINE) -masm=intel -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone $(OPT_FLAGS)
export LDFLAGS_COMMON := -nostdlib -z max-page-size=0x1000

ifeq ($(use_bios),1)

export BOOT_ASFLAGS_STAGE1 := -f bin

export BOOT_ASFLAGS_STAGE2 := $(COMMON_INCLUDE) $(ASFLAGS_COMMON) -f elf32
export BOOT_CFLAGS_STAGE2 := $(COMMON_INCLUDE) $(CFLAGS_COMMON)
export BOOT_LDFLAGS_STAGE2 := $(LDFLAGS_COMMON)

endif

export TARGET_ASFLAGS := $(COMMON_INCLUDE) $(ASFLAGS_COMMON) -f elf64
export TARGET_CFLAGS := -mcmodel=kernel $(COMMON_INCLUDE) $(CFLAGS_COMMON)
export TARGET_CXXFLAGS := $(COMMON_INCLUDE) $(CFLAGS_COMMON)
export TARGET_LDFLAGS := $(LDFLAGS_COMMON)

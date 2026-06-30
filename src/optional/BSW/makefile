USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS += -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -Ilibc -fno-omit-frame-pointer -I $(INCLUDE_DIR) -I. -I $(SOURCE_DIR)/src/libs

BUILD_DIRBSW = $(abspath $(BUILD_DIR)/BSW)
BUILD_DIRC = $(BUILD_DIRBSW)/c
BUILD_DIRASM = $(BUILD_DIRBSW)/asm

HEADERS_C = $(shell find . -type f -name "*.h" -not -path "./arch/*") \
			$(shell find ${SOURCE_ARCH} -type f -name "*.h")

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.c"))

OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

HEADERS_ASM = $(shell find . -type f -name "*.inc" -not -path "./arch/*") \
			  $(shell find ${SOURCE_ARCH} -type f -name "*.inc")

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.asm"))

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

.PHONY: all BSW clean always

all: BSW

BSW: $(BUILD_DIRBSW)/BSW.dll

$(BUILD_DIRBSW)/BSW.dll: $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@$(TARGET_AR) --target=pe-$(arch) rcs $@ $^
	@echo "--> Created:  BSW.dll"

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(KERNEL_TARGET_CFLAGS) -c -o $@ $<
#	@$(TARGET_CC) $(KERNEL_TARGET_CFLAGS) -c -save-temps -o $@.c $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(KERNEL_TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

always:
	@mkdir -p $(BUILD_DIRASM)
	@mkdir -p $(BUILD_DIRC)

clean:
	rm -f $(BUILD_DIRBSW)/kernel.elf

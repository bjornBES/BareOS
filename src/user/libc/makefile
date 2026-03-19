
TARGET_ASMFLAGS := -f elf
TARGET_CFLAGS := -m32 -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -Ilibc -fno-omit-frame-pointer

BUILD_DIRC = $(BUILD)/libc/c
BUILD_DIRASM = $(BUILD)/libc/asm

HEADERS_C = $(shell find . -type f -name "*.h")
SOURCES_C := $(shell find . -type f -name "*.c")
OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

HEADERS_ASM = $(shell find . -type f -name "*.inc")
SOURCES_ASM := $(shell find . -type f -name "*.asm")
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

all: $(BUILD)/libc.a

$(BUILD)/libc.a: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_AR) rcs $@ $^

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<



USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS += -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -Ilibc -fno-omit-frame-pointer -I $(INCLUDE_DIR) -I. -I $(SOURCE_DIR)/src/libs

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
	@echo "--> Created:  libc.a"

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(USER_TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(USER_TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

always:

clean:
	rm -f $(BUILD)/libc.a

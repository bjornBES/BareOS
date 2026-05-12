
USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS += -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -fno-omit-frame-pointer
USER_TARGET_LINKFLAGS +=
USER_TARGET_LIBS +=

BUILD_DIRC = $(BUILD)/c
BUILD_DIRASM = $(BUILD)/asm

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c"))
OBJECTS_C := $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm"))
OBJECTS_ASM := $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

all: always $(BUILD)/init.elf

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(USER_TARGET_CFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(USER_TARGET_ASMFLAGS) $< -o $@
	@echo "--> Compiled: " $<

$(BUILD)/init.elf: $(OBJECTS_C) $(OBJECTS_ASM)
	@$(TARGET_LD) $(USER_TARGET_LINKFLAGS) -Wl,-Map=$(BUILD)/init.map -o $@ $^ $(USER_TARGET_LIBS)

always:

clean:
	rm -f $(BUILD)/init.elf
USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS += -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -Ilibc -fno-omit-frame-pointer
USER_TARGET_LINKFLAGS += -nostdlib -T linker.ld
USER_TARGET_LIBS :=

BUILD   := $(BUILD_DIR)/user
LIBC    := $(BUILD)/libc.a

BUILD_DIRC = $(BUILD)/c
BUILD_DIRASM = $(BUILD)/asm

SOURCES_C := $(patsubst ./%,%,$(shell find init/. -type f -name "*.c"))
OBJECTS_C := $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

SOURCES_ASM := $(patsubst ./%,%,$(shell find init/. -type f -name "*.asm"))
OBJECTS_ASM := $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

LIBS = $(LIBC)

.PHONY: all clean always program

all: always $(LIBC) program

$(LIBC):
	@$(MAKE) -C libc BUILD=$(abspath $(BUILD))

program: $(BUILD)/init.elf

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	$(TARGET_CC) $(USER_TARGET_CFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	$(TARGET_ASM) $(USER_TARGET_ASMFLAGS) $< -o $@
	@echo "--> Compiled: " $<

$(BUILD)/init.elf: $(OBJECTS_C) $(OBJECTS_ASM) $(LIBC)
	$(TARGET_LD) $(USER_TARGET_LINKFLAGS) -Wl,-Map=$(BUILD)/init.map -o $@ $^ $(USER_TARGET_LIBS)


always:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -m32 -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -Ilibc -fno-omit-frame-pointer
TARGET_LINKFLAGS += -nostdlib -T linker.ld
TARGET_LIBS := -lgcc

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
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $< -o $@
	@echo "--> Compiled: " $<

$(BUILD)/init.elf: $(OBJECTS_C) $(OBJECTS_ASM) $(LIBC)
	$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD)/init.map -o $@ $^ $(TARGET_LIBS)


always:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

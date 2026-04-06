
TARGET_ASMFLAGS := -f elf
TARGET_CFLAGS += -m32 -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -Ilibc -fno-omit-frame-pointer
TARGET_LINKFLAGS += -nostdlib -T linker.ld
TARGET_LIBS := -lgcc

BUILD_DIRC = $(BUILD)/c
BUILD_DIRASM = $(BUILD)/asm

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c"))
OBJECTS_C := $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm"))
OBJECTS_ASM := $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

all: always $(BUILD)/hello.elf

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	$(TARGET_ASM) -f elf32 $< -o $@
	@echo "--> Compiled: " $<

$(BUILD)/hello.elf: $(OBJECTS_C) $(OBJECTS_ASM) $(LIBC)
	$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD)/hello.map -o $@ $^ $(TARGET_LIBS)

always:
	mkdir -p $(BUILD)
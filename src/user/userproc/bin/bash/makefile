
USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS += -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -fno-omit-frame-pointer -I $(INCLUDE_DIR) -I. -I $(SOURCE_DIR)/src/libs -I ../../libc
USER_TARGET_LINKFLAGS +=
USER_TARGET_LIBS +=

BUILD_DIRC = $(BUILD)/bash/c
BUILD_DIRASM = $(BUILD)/bash/asm

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c"))
OBJECTS_C := $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm"))
OBJECTS_ASM := $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

all: always $(BUILD)/bash.elf

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(USER_TARGET_CFLAGS) -c $< -o $@
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(USER_TARGET_ASMFLAGS) $< -o $@
	@echo "--> Compiled: " $<

$(BUILD)/bash.elf: $(OBJECTS_C) $(OBJECTS_ASM)
	@$(TARGET_LD) $(USER_TARGET_LINKFLAGS) -Wl,-Map=$(BUILD)/bash.map -o $@ $^ $(BUILD)/libc.a $(USER_TARGET_LIBS)

always:
	@mkdir -p $(BUILD)

clean:
	rm -f $(BUILD)/bash.elf
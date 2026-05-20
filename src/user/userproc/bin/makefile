USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS := -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -fno-omit-frame-pointer -I $(INCLUDE_DIR) -I $(SOURCE_DIR)/src/libs
USER_TARGET_LINKFLAGS +=
USER_TARGET_LIBS += -lgcc

BUILD_DIRC = $(BUILD)/c
BUILD_DIRASM = $(BUILD)/asm

SOURCES_C := $(shell find . -maxdepth 1 -name "*.c")
BINS := $(patsubst %.c, $(BUILD)/%.elf, $(SOURCES_C))

all: always $(BINS)
	@echo "BINS" $(BINS)
	@$(MAKE) -C bash BUILD=$(abspath $(BUILD))
	@$(MAKE) -C BASIC BUILD=$(abspath $(BUILD))

$(BUILD)/%.elf: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(USER_TARGET_CFLAGS) -c $< -o $@ -lc
	@echo "--> Compiled: " $<

always:
	mkdir -p $(BUILD)

clean:
	rm -f $(BINS)

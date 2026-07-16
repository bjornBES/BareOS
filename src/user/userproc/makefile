USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS += -std=c99 -ffreestanding -nostdlib -nostdinc \
           		 -Wall -Wextra -Ilibc -fno-omit-frame-pointer -I $(INCLUDE_DIR) -I. -I $(SOURCE_DIR)/src/libs -I ../libc
USER_TARGET_LINKFLAGS += -nostdlib -T $(abspath linker.ld)
USER_TARGET_LIBS += $(LIBS)

BUILD   = $(BUILD_DIR)/user
LIBC    = $(BUILD)/libc.a
INIT 	= $(BUILD)/init.elf

BUILD_DIRC = $(BUILD)/c
BUILD_DIRASM = $(BUILD)/asm

.PHONY: all clean always bin

all: $(LIBC) $(INIT) bin

$(LIBC):
	@$(MAKE) -C libc BUILD=$(abspath $(BUILD))

bin: $(LIBC)
	$(MAKE) -C bin BUILD=$(abspath $(BUILD))

$(INIT): $(LIBC)
	@$(MAKE) -C init BUILD=$(abspath $(BUILD))


always:
	mkdir -p $(BUILD)
	@$(MAKE) -C libc always BUILD=$(abspath $(BUILD))
	@$(MAKE) -C init always BUILD=$(abspath $(BUILD))
	@$(MAKE) -C bin always BUILD=$(abspath $(BUILD))

clean:
	@$(MAKE) -C libc clean BUILD=$(abspath $(BUILD))
	@$(MAKE) -C init clean BUILD=$(abspath $(BUILD))
	@$(MAKE) -C bin clean BUILD=$(abspath $(BUILD))
	rm -rf $(BUILD)

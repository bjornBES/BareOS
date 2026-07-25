.PHONY: all bootloader clean always

all: bootloader

bootloader:
ifeq ($(use_bios),1)
	@$(MAKE) -C bios BUILD_DIR=$(abspath $(BUILD_DIR))
else
endif

always:
ifeq ($(use_bios),1)
	@$(MAKE) -C bios BUILD_DIR=$(abspath $(BUILD_DIR)) always
else
endif

clean:
ifeq ($(use_bios),1)
	@$(MAKE) -C bios BUILD_DIR=$(abspath $(BUILD_DIR)) clean
else
endif

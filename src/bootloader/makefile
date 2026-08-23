.PHONY: all bootloader clean always

all: bootloader

bootloader:
ifeq ($(use_bios),1)
	@$(MAKE) -C bios BUILD_DIR=$(abspath $(BUILD_DIR))
else
	@echo TODO UEFI
endif

always:
ifeq ($(use_bios),1)
	@$(MAKE) -C bios BUILD_DIR=$(abspath $(BUILD_DIR)) always
else
	@echo TODO UEFI
endif

clean:
ifeq ($(use_bios),1)
	@$(MAKE) -C bios BUILD_DIR=$(abspath $(BUILD_DIR)) clean
else
	@echo TODO UEFI
endif

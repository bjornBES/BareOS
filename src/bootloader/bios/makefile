.PHONY: all bootloader clean always

all: bootloader

bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin:
	@$(MAKE) -C stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin:
	@$(MAKE) -C stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

always:
	@$(MAKE) -C stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) always
	@$(MAKE) -C stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) always

clean:
	@$(MAKE) -C stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean

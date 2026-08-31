
include mk/config.mk

include mk/toolchain_config.mk

.PHONY: all build_image bootloader clean always debug libs kernel
# user

all: build_image

include mk/toolchain.mk

#
# Floppy image
#
build_image: $(BUILD_DIR)/image.iso

$(BUILD_DIR)/image.iso: bootloader kernel
# user
	@python ./scripts/python/make_image.py

	@echo "--> Created: image.iso"

#
# Bootloader
#
bootloader: config build_bootloader

build_bootloader:
	@$(MAKE) -C src/bootloader BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: libs config $(BUILD_DIR)/kernel/kernel.elf


$(BUILD_DIR)/kernel/kernel.elf: always
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

config/config.env:

config: src/libs/include/config.h

src/libs/include/config.h: config/config.env scripts/gen_config.sh
	sh scripts/gen_config.sh config/config.env $@ src/libs/include/config.inc


libs: $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libcore.a:
	@$(MAKE) -C src/libs TARGET_ARCH=64 BUILD_DIR=$(abspath $(BUILD_DIR)) -s




#
# user
#
# user: $(TARGET_CORE_LIBS)
# 	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR))

runnow:
	python ./scripts/python/run_vm.py
run: $(BUILD_DIR)/image.iso
	python ./scripts/python/run_vm.py

debug: clean all

	@echo "running debug"
	python ./scripts/python/run_vm.py debug

debugnow:
	python ./scripts/python/run_vm.py debug

# menuconfig-%:
# 	$(MAKE) -C src/user/userland menuconfig-$*

# userland-install:
# 	$(MAKE) -C src/user/userland install BUILD_DIR=$(abspath $(BUILD_DIR))

# userland:
# 	$(MAKE) -C src/user/userland BUILD_DIR=$(abspath $(BUILD_DIR))
# userland-clean:
# 	$(MAKE) -C src/user/userland clean BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C src/bootloader BUILD_DIR=$(abspath $(BUILD_DIR)) always -s
# 	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) always -s
# 	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR)) always

#
# Clean
#
clean:
	@$(MAKE) -C src/bootloader BUILD_DIR=$(abspath $(BUILD_DIR)) clean
# 	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
# 	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*
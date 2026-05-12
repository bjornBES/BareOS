include mk/config.mk

.PHONY: all floppy_image bootloader clean always debug libs kernel user

all: floppy_image

include mk/toolchain.mk

#
# Floppy image
#
floppy_image: $(BUILD_DIR)/image.iso

$(BUILD_DIR)/image.iso: bootloader kernel user
	@bash ./scripts/make_disk.sh $(imageType) $(imageFS) $(imageSize) $(arch) $(config)

	@echo "--> Created: " $(floppyOutput)

#
# Bootloader
#
bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C src/Bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: libs $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	@$(MAKE) -C src/Bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: libs $(BUILD_DIR)/kernel/kernel.elf


$(BUILD_DIR)/kernel/kernel.elf: always
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

libs: $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libcore.a:
	@$(MAKE) -C src/libs BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat
$(BUILD_DIR)/tools/fat: always tools/fat/fat.c
	@mkdir -p $(BUILD_DIR)/tools
	@$(MAKE) -C tools/fat BUILD_DIR=$(abspath $(BUILD_DIR))

#
# user
#
user: $(TARGET_CORE_LIBS)
	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR))

runnow:
	python tools/run_vm.py
run: $(BUILD_DIR)/image.iso
	bash scripts/run.sh disk $(arch) $(BUILD_DIR)/image.iso
#	python tools/run_vm.py
debug_flags:
	@echo "add -g"
	$(eval KERNEL_TARGET_ASMFLAGS += -g)
	$(eval KERNEL_TARGET_CFLAGS += -g)
	$(eval TARGET_ASMFLAGS += -g)
	$(eval TARGET_CFLAGS += -g)
	$(eval ASMFLAGS += -g)
	$(eval CFLAGS += -g)

debug: debug_flags clean all

	@echo "running debug"
	bash scripts/debug.sh disk $(arch) $(BUILD_DIR)/image.iso $(BUILD_DIR)/kernel/kernel.elf

debugnow:
	bash scripts/debug.sh disk $(arch) $(BUILD_DIR)/image.iso $(BUILD_DIR)/kernel/kernel.elf

menuconfig-%:
	$(MAKE) -C src/user/userland menuconfig-$*

install:
	$(MAKE) -C src/user/userland install

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C src/Bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) always
	@$(MAKE) -C src/Bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) always
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) always
	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR)) always

toolchain:

#
# Clean
#
clean:
	@$(MAKE) -C src/Bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/Bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*
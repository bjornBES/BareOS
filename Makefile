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

	@echo "--> Created: $(floppyOutput)"

#
# Bootloader
#
bootloader: libs src/bootloader/bios/stage2/include/config.h build_bootloader

build_bootloader:
	@$(MAKE) -C src/bootloader BUILD_DIR=$(abspath $(BUILD_DIR)) -s

#
# Kernel
#
kernel: libs src/kernel/include/kernel/config.h $(BUILD_DIR)/kernel/kernel.elf


$(BUILD_DIR)/kernel/kernel.elf: always
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) -s

config/config.env:

src/kernel/include/kernel/config.h: config/config.env scripts/gen_config.sh
	sh scripts/gen_config.sh config/config.env $@ src/kernel/include/kernel/config.inc

src/bootloader/bios/stage2/include/config.h: config/config.env scripts/gen_config.sh
	sh scripts/gen_config.sh config/config.env $@ src/bootloader/bios/stage2/include/config.inc

libs: $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libcore.a:
	@$(MAKE) -C src/libs BUILD_DIR=$(abspath $(BUILD_DIR)) -s

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat
$(BUILD_DIR)/tools/fat: always tools/fat/fat.c
	@mkdir -p $(BUILD_DIR)/tools
	@$(MAKE) -C tools/fat BUILD_DIR=$(abspath $(BUILD_DIR)) -s

#
# user
#
user: $(TARGET_CORE_LIBS)
	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR))

runnow:
#	python tools/run_vm.py
	python tools/run_vm.py
# 	bash scripts/run.sh disk $(arch) $(BUILD_DIR)/image.iso
run: $(BUILD_DIR)/image.iso
	python tools/run_vm.py
# 	bash scripts/run.sh disk $(arch) $(BUILD_DIR)/image.iso
#	python tools/run_vm.py
debug_flags:
	@echo "add -g"
	$(eval KERNEL_TARGET_ASMFLAGS += -g)
	$(eval KERNEL_TARGET_CFLAGS += -g)
	$(eval KERNEL_TARGET_CXXFLAGS += -g)

	$(eval USER_TARGET_ASMFLAGS += -g)
	$(eval USER_TARGET_CFLAGS += -g)
	$(eval USER_TARGET_CXXFLAGS += -g)

	$(eval TARGET_ASMFLAGS += -g)
	$(eval TARGET_CFLAGS += -g)
	$(eval TARGET_CXXFLAGS += -g)

	$(eval ASMFLAGS += -g)
	$(eval CFLAGS += -g)

debug: debug_flags clean all

	@echo "running debug"
	python tools/run_vm.py debug
# 	bash scripts/debug.sh disk $(arch) $(BUILD_DIR)/image.iso $(BUILD_DIR)/kernel/kernel.elf

debugnow:
	python tools/run_vm.py debug
# 	bash scripts/debug.sh disk $(arch) $(BUILD_DIR)/image.iso $(BUILD_DIR)/kernel/kernel.elf

menuconfig-%:
	$(MAKE) -C src/user/userland menuconfig-$*

userland-install:
	$(MAKE) -C src/user/userland install BUILD_DIR=$(abspath $(BUILD_DIR))

userland:
	$(MAKE) -C src/user/userland BUILD_DIR=$(abspath $(BUILD_DIR))
userland-clean:
	$(MAKE) -C src/user/userland clean BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)
	@$(MAKE) -C src/bootloader BUILD_DIR=$(abspath $(BUILD_DIR)) always -s
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) always -s
	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR)) always

#
# Clean
#
clean:
	@$(MAKE) -C src/bootloader BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*
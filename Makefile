include scripts/config.mk

.PHONY: all floppy_image bootloader clean always debug libs kernel

all: floppy_image

include scripts/toolchain.mk

#
# Floppy image
#
floppy_image: $(BUILD_DIR)/image.img

$(BUILD_DIR)/image.img: bootloader kernel user
	@bash ./scripts/make_disk.sh $(imageType) $(imageFS) $(imageSize) $(arch) $(config)

	@echo "--> Created: " $(floppyOutput)

#
# Bootloader
#
bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C src/Bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: libs bioslib $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	@$(MAKE) -C src/Bootloader/stage2 -j 4 BUILD_DIR=$(abspath $(BUILD_DIR))

# stage2_menu: libs bioslib $(ROOT_DIR)/root/blmenu.elf

# $(ROOT_DIR)/root/blmenu.elf: always
# 	@$(MAKE) -C src/Bootloader/stage2_menu -j 4 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: libs $(BUILD_DIR)/kernel/kernel.elf


$(BUILD_DIR)/kernel/kernel.elf: always
	@$(MAKE) -C src/kernel -j 4 BUILD_DIR=$(abspath $(BUILD_DIR))

libs: $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libcore.a:
	@$(MAKE) -C src/libs BUILD_DIR=$(abspath $(BUILD_DIR))

bioslib: $(BUILD_DIR)/libbios.a

$(BUILD_DIR)/libbios.a:
	@$(MAKE) -C src/BIOS BUILD_DIR=$(abspath $(BUILD_DIR))

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
user: $(TARGET_LIBS)
	@$(MAKE) -C src/user BUILD_DIR=$(abspath $(BUILD_DIR))

runnow:
	bash run.sh disk $(BUILD_DIR)/image.img
run: $(BUILD_DIR)/image.img
	bash run.sh disk $(BUILD_DIR)/image.img
debug_flags:
	@echo "add -g"
	$(eval TARGET_ASM += -g)
	$(eval TARGET_CFLAGS += -g)

debug: debug_flags clean all

	@echo "running debug"
	bash debug.sh disk $(BUILD_DIR)/image.img $(BUILD_DIR)/floppyImage.img $(BUILD_DIR)/sataImage.img

debugnow:
	bash debug.sh disk $(BUILD_DIR)/image.img $(BUILD_DIR)/floppyImage.img $(BUILD_DIR)/sataImage.img


load: $(BUILD_DIR)/image.img
	@bash ./scripts/image/LoadImage.sh $(BUILD_DIR)/image.img /dev/sdd

#
# Always
#
always:
#	@echo "mkdir -p $(BUILD_DIR)" 
#	@mkdir -p $(BUILD_DIR)

toolchain:

#
# Clean
#
clean:
	@$(MAKE) -C src/Bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/Bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
#	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*
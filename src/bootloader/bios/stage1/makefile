BOOT_LDFLAGS_STAGE1 += $(LINKFLAGS) -nostdlib


.PHONY: all clean

all: stage1

BUILD_DIRSTAGE1 = $(BUILD_DIR)/stage1
BUILD_DIRASM = $(BUILD_DIRSTAGE1)/asm


MBR_SOURCE = mbr.asm
MBR_OBJ = $(BUILD_DIRASM)/mbr.obj
MBR_BIN = $(BUILD_DIRSTAGE1)/mbr.bin
MBR_MAP = $(BUILD_DIRSTAGE1)/mbr.map

# VBR bootloader
VBR_SOURCE = vbr.asm
VBR_OBJ = $(BUILD_DIRASM)/vbr.obj
VBR_BIN = $(BUILD_DIRSTAGE1)/vbr.bin
VBR_MAP = $(BUILD_DIRSTAGE1)/vbr.map

stage1: $(MBR_BIN) $(VBR_BIN)

# MBR linker target
$(MBR_BIN): $(MBR_OBJ)
	@$(BOOT_LD) $(BOOT_LDFLAGS_STAGE1) -T linker_mbr.ld -Wl,-Map=$(MBR_MAP) -o $@ $<
	@echo "$(LD_PREFIX)" $@

# VBR linker target
$(VBR_BIN): $(VBR_OBJ)
	@$(BOOT_LD) $(BOOT_LDFLAGS_STAGE1) -T linker_vbr.ld -Wl,-Map=$(VBR_MAP) -o $@ $<
	@echo "$(LD_PREFIX)" $@

# Assembly compilation
$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(BOOT_AS) -f elf $< -DFILESYSTEM=$(imageFS) -o $@
	@echo "$(AS_PREFIX)" $<

always:

clean:
	@rm -f $(BUILD_DIRSTAGE1)/*.bin
	@rm -f $(BUILD_DIRSTAGE1)/*.map

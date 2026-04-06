TARGET16_LINKFLAGS += $(LINKFLAGS) -nostdlib


.PHONY: all clean

all: stage1

BUILD_DIRSTAGE1 = $(BUILD_DIR)/stage1
BUILD_DIRASM = $(BUILD_DIRSTAGE1)/asm

# MBR bootloader
# MBR_SOURCE = boot.asm
# MBR_OBJ = $(BUILD_DIRASM)/boot.obj
# MBR_BIN = $(BUILD_DIRSTAGE1)/stage1.bin
# MBR_MAP = $(BUILD_DIRSTAGE1)/stage1.map

MBR2_SOURCE = mbr.asm
MBR2_OBJ = $(BUILD_DIRASM)/mbr.obj
MBR2_BIN = $(BUILD_DIRSTAGE1)/mbr.bin
MBR2_MAP = $(BUILD_DIRSTAGE1)/mbr.map

# VBR bootloader
VBR_SOURCE = vbrBoot.asm
VBR_OBJ = $(BUILD_DIRASM)/vbrBoot.obj
VBR_BIN = $(BUILD_DIRSTAGE1)/vbr.bin
VBR_MAP = $(BUILD_DIRSTAGE1)/vbr.map

stage1: $(MBR2_BIN) $(VBR_BIN)

# MBR linker target
$(MBR_BIN): $(MBR_OBJ)
#	@echo "Linking MBR: $(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(MBR_MAP) -o $@ $<"
	$(TARGET32_LD) $(TARGET16_LINKFLAGS) -T linker.ld -Wl,-Map=$(MBR_MAP) -o $@ $<
	@echo "--> Created MBR: $@"

# MBR linker target
$(MBR2_BIN): $(MBR2_OBJ)
#	@echo "Linking MBR: $(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(MBR2_MAP) -o $@ $<"
	$(TARGET32_LD) $(TARGET16_LINKFLAGS) -T linker.ld -Wl,-Map=$(MBR2_MAP) -o $@ $<
	@echo "--> Created MBR: $@"

# VBR linker target
$(VBR_BIN): $(VBR_OBJ)
#	@echo "Linking VBR: $(TARGET_LD) -T linker_vbr.ld -nostdlib -Wl,-Map=$(VBR_MAP) -o $@ $<"
	$(TARGET32_LD) $(TARGET16_LINKFLAGS) -T linker_vbr.ld -Wl,-Map=$(VBR_MAP) -o $@ $<
	@echo "--> Created VBR: $@"

# Assembly compilation
$(BUILD_DIRASM)/%.obj: %.asm
#	@echo "$(TARGET_ASM) $(TARGET_ASMFLAGS) $< -DFILESYSTEM=$(imageFS) -o $@"
	@mkdir -p $(@D)
	@$(TARGET32_ASM) -f elf $< -DFILESYSTEM=$(imageFS) -o $@

clean:
	@rm -f $(BUILD_DIRSTAGE1)/*.bin
	@rm -f $(BUILD_DIRSTAGE1)/*.map

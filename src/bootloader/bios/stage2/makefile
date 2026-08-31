BOOT_ASFLAGS_STAGE2 +=
BOOT_CFLAGS_STAGE2 +=
BOOT_LDFLAGS_STAGE2 +=

BUILD_DIRSTAGE2 = $(BUILD_DIR)/stage2
BUILD_DIRC = $(BUILD_DIRSTAGE2)/c
BUILD_DIRASM = $(BUILD_DIRSTAGE2)/asm

HEADERS_C = $(shell find . -type f -name "*.h" -not -path "./arch/*" -not -path "./modules/*") \
			$(shell find ${SOURCE_ARCH} -type f -name "*.h") \
			$(shell find ./arch/include -type f -name "*.h")

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c" -not -path "./arch/*" -not -path "./modules/*") \
	$(shell find ${SOURCE_ARCH} -type f -name "*.c") \
	$(shell find ./arch/include -type f -name "*.c"))


HEADERS_ASM = $(shell find . -type f -name "*.inc" -not -path "./arch/*" -not -path "./modules/*") \
			$(shell find ${SOURCE_ARCH} -type f -name "*.inc") \
			$(shell find ./arch/include -type f -name "*.inc")

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm" -not -path "./arch/*" -not -path "./modules/*") \
	$(shell find ${SOURCE_ARCH} -type f -name "*.asm") \
	$(shell find ./arch/include -type f -name "*.asm"))

OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))


.PHONY: all stage2 clean always

all: stage2

stage2: $(BUILD_DIRSTAGE2)/stage2.bin $(BUILD_DIRSTAGE2)/stage2.elf

$(BUILD_DIRSTAGE2)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(BOOT_LD) -T linker.ld $(BOOT_LDFLAGS_STAGE2) -Wl,-Map=$(BUILD_DIRSTAGE2)/stage2.map -o $@ $^ $(COMMON_LIBS)
	@echo "$(LD_PREFIX) stage2.bin"

$(BUILD_DIRSTAGE2)/stage2.elf: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(BOOT_LD) -T linker_debug.ld $(BOOT_LDFLAGS_STAGE2) -o $@ $^ $(COMMON_LIBS)
	@echo "$(LD_PREFIX) stage2.elf"

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(BOOT_CC) $(BOOT_CFLAGS_STAGE2) -c -o $@ $<
# 	@$(BOOT_CC) $(BOOT_CFLAGS_STAGE2) --save-temps -c -o $@ $<
	@echo "$(CC_PREFIX)" $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(BOOT_AS) $(BOOT_ASFLAGS_STAGE2) -o $@ $<
	@echo "$(AS_PREFIX)" $<

always:
	@mkdir -p $(BUILD_DIRASM)
	@mkdir -p $(BUILD_DIRC)


clean:
	@rm -f $(BUILD_DIRSTAGE2)/stage2.bin

TARGET_ASFLAGS += 
TARGET_CFLAGS += -I$(SOURCE_DIR)/src/kernel/arch/$(SRCARCH)
TARGET_LDFLAGS += 

ifeq ($(arch),i686)
LINKER_SCRIPT += linker32.ld
endif
ifeq ($(arch),x86_64)
LINKER_SCRIPT += linker64.ld
endif

BUILD_DIR_KERNEL = $(BUILD_DIR)/kernel
BUILD_DIR_C = $(BUILD_DIR_KERNEL)/c
BUILD_DIR_ASM = $(BUILD_DIR_KERNEL)/asm
BUILD_DIR_CBL = $(BUILD_DIR_KERNEL)/cobol
TEMP_DIR_CBL = $(BUILD_DIR_KERNEL)/cobol/temp

HEADERS_C = $(patsubst ./%,%,$(shell find . -type f -name "*.h" -not -path "./arch/*" -not -path "./modules/*") \
	$(shell find ${SOURCE_ARCH} -type f -name "*.h") \
	$(shell find ./arch/generic -type f -name "*.h") \
	$(shell find ./arch/include -type f -name "*.h"))

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c" -not -path "./arch/*" -not -path "./modules/*") \
	$(shell find ${SOURCE_ARCH} -type f -name "*.c") \
	$(shell find ./arch/generic -type f -name "*.c") \
	$(shell find ./arch/include -type f -name "*.c"))

OBJECTS_C = $(patsubst %.c, $(BUILD_DIR_C)/%.obj, $(SOURCES_C))

HEADERS_ASM = $(patsubst ./%,%,$(shell find . -type f -name "*.inc" -not -path "./arch/*" -not -path "./modules/*") \
	$(shell find ${SOURCE_ARCH} -type f -name "*.inc") \
	$(shell find ./arch/generic -type f -name "*.inc") \
	$(shell find ./arch/include -type f -name "*.inc"))

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm" -not -path "./arch/*" -not -path "./modules/*") \
	$(shell find ${SOURCE_ARCH} -type f -name "*.asm") \
	$(shell find ./arch/generic -type f -name "*.asm") \
	$(shell find ./arch/include -type f -name "*.asm"))

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR_ASM)/%.obj, $(SOURCES_ASM))


.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIR_KERNEL)/kernel.elf

$(BUILD_DIR_KERNEL)/kernel.elf: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) -T $(LINKER_SCRIPT) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR_KERNEL)/kernel.map -o $@ $^ $(COMMON_LIBS) $(BUILD_DIR)/libcore.a
	@echo "$(LD_PREFIX) kernel.elf"

$(BUILD_DIR_C)/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) -m64 $(TARGET_CFLAGS) -c -o $@ $<
# 	@$(TARGET_CC) $(TARGET_CFLAGS) -c -save-temps -o $@.c $<
	@echo "$(CC_PREFIX)" $<

$(BUILD_DIR_ASM)/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_AS) $(TARGET_ASFLAGS) -o $@ $<
	@echo "$(AS_PREFIX)" $<

always:
	@mkdir -p $(BUILD_DIR_ASM)
	@mkdir -p $(BUILD_DIR_C)
	@mkdir -p $(BUILD_DIR_CBL)
	@mkdir -p $(TEMP_DIR_CBL)

arch/x86/isrs_gen.c arch/x86/isrs_gen.inc:
	@$(SOURCE_DIR)/build_tools/generate_isrs.sh $@

clean:
	@rm -f $(BUILD_DIR_KERNEL)/kernel.elf
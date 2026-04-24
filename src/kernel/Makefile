KERNEL_TARGET_ASMFLAGS +=
KERNEL_TARGET_CFLAGS += -ffreestanding -nostdlib -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I. -I $(SOURCE_DIR)/src/libs -I $(INCLUDE_DIR) -D__$(arch)__=1
KERNEL_TARGET_LINKFLAGS += -nostdlib
KERNEL_TARGET_LIBS +=

ifeq ($(arch),i686)
KERNEL_TARGET_LINKFLAGS += -T linker32.ld
KERNEL_TARGET_ASMFLAGS += -D__x86_64__=0
endif
ifeq ($(arch),x86_64)
KERNEL_TARGET_LINKFLAGS += -T linker64.ld
KERNEL_TARGET_ASMFLAGS += -D__i686__=0
endif

BUILD_DIRKERNEL = $(BUILD_DIR)/kernel
BUILD_DIRC = $(BUILD_DIRKERNEL)/c
BUILD_DIRASM = $(BUILD_DIRKERNEL)/asm
BUILD_DIRCBL = $(BUILD_DIRKERNEL)/cobol
TEMP_DIRCBL = $(BUILD_DIRKERNEL)/cobol/temp

HEADERS_C = $(shell find . -type f -name "*.h" -not -path "./arch/*") \
			$(shell find ${SOURCE_ARCH} -type f -name "*.h")

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.c"))

OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

HEADERS_ASM = $(shell find . -type f -name "*.inc" -not -path "./arch/*") \
			  $(shell find ${SOURCE_ARCH} -type f -name "*.inc")

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.asm"))

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

LIBS = $(BUILD_DIR)/libcore.a

.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIRKERNEL)/kernel.elf

$(BUILD_DIRKERNEL)/kernel.elf: $(OBJECTS_ASM) $(OBJECTS_C) $(LIBS)
	@$(TARGET_LD) $(KERNEL_TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIRKERNEL)/kernel.map -o $@ $^ $(KERNEL_TARGET_LIBS)
	@echo "--> Created:  kernel.elf"

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(KERNEL_TARGET_CFLAGS) -c -o $@ $<
#	@$(TARGET_CC) $(KERNEL_TARGET_CFLAGS) -c -save-temps -o $@.c $<
	@echo "--> Compiled: " $<

$(BUILD_DIRASM)/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(KERNEL_TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

always:
	@if not exist $(BUILD_DIRASM) mkdir $(BUILD_DIRASM)
	@if not exist $(BUILD_DIRC) mkdir $(BUILD_DIRC)
	@if not exist $(BUILD_DIRCBL) mkdir $(BUILD_DIRCBL)
	@if not exist $(TEMP_DIRCBL) mkdir $(TEMP_DIRCBL)

# arch/x86/isrs_gen.c arch/x86/isrs_gen.inc:
# @$(SOURCE_DIR)/build_tools/generate_isrs.sh $@

clean:
	rm -f $(BUILD_DIRKERNEL)/kernel.elf



BUILD_DIRC = $(BUILD_DIR)/libc/c
BUILD_DIRASM = $(BUILD_DIR)/libc/asm

HEADERS_C = $(shell find . -type f -name "*.h")
SOURCES_C := $(shell find . -type f -name "*.c")
OBJECTS_C = $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

HEADERS_ASM = $(shell find . -type f -name "*.inc")
SOURCES_ASM := $(shell find . -type f -name "*.asm")
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

all: $(BUILD_DIR)/libc.a

$(BUILD_DIR)/libc.a: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(USER_TARGET_AR) rcs $@ $^
	@echo "$(AR_PREFIX) libc.a"

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(USER_TARGET_CC) $(USER_TARGET_CFLAGS) -c -o $@ $<
	@echo "$(CC_PREFIX)" $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(USER_TARGET_AS) $(USER_TARGET_ASFLAGS) -o $@ $<
	@echo "$(ASM_PREFIX)" $<

always:

clean:
	@rm -f $(BUILD_DIR)/libc.a

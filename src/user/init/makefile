

BUILD_DIRC = $(BUILD_DIR)/user/c
BUILD_DIRASM = $(BUILD_DIR)/user/asm

SOURCES_C := $(patsubst ./%,%,$(shell find . -type f -name "*.c"))
OBJECTS_C := $(patsubst %.c, $(BUILD_DIRC)/%.obj, $(SOURCES_C))

SOURCES_ASM := $(patsubst ./%,%,$(shell find . -type f -name "*.asm"))
OBJECTS_ASM := $(patsubst %.asm, $(BUILD_DIRASM)/%.obj, $(SOURCES_ASM))

all: always $(BUILD_DIR)/user/init.elf

$(BUILD_DIRC)/%.obj: %.c
	@mkdir -p $(@D)
	@$(USER_TARGET_CC) $(USER_TARGET_CFLAGS) -c $< -o $@
# 	@$(TARGET_CC) $(USER_TARGET_CFLAGS) -c -save-temps $< -o $@
	@echo "$(CC_PREFIX)" $<

$(BUILD_DIRASM)/%.obj: %.asm
	@mkdir -p $(@D)
	@$(USER_TARGET_AS) $(USER_TARGET_ASFLAGS) $< -o $@
	@echo "$(ASM_PREFIX)" $<

$(BUILD_DIR)/user/init.elf: $(OBJECTS_C) $(OBJECTS_ASM)
	@$(USER_TARGET_LD) $(USER_TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/user/init.map -o $@ $^ $(USER_TARGET_LIBS)
	@echo "$(LD_PREFIX) init.elf"

always:

clean:
	@rm -f $(BUILD_DIR)/user/init.elf
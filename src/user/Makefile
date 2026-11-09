.PHONY: all clean always

all: init_proc libc

libc: $(BUILD_DIR)/libc.a

$(BUILD_DIR)/libc.a:
	@$(MAKE) -C libc BUILD_DIR=$(abspath $(BUILD_DIR))

init_proc: libc
	@$(MAKE) -C init BUILD_DIR=$(abspath $(BUILD_DIR))

always:
	@$(MAKE) -C libc always BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C init always BUILD_DIR=$(abspath $(BUILD_DIR))

clean:
	@$(MAKE) -C libc clean BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C init clean BUILD_DIR=$(abspath $(BUILD_DIR))

.PHONY: all clean always userland userproc

all: userland userproc

userproc:
	@$(MAKE) -C userproc BUILD_DIR=$(abspath $(BUILD_DIR))

userland:
#	$(MAKE) -C userland BUILD_DIR=$(abspath $(BUILD_DIR))

always:
	$(MAKE) -C userproc always BUILD_DIR=$(abspath $(BUILD_DIR))
	$(MAKE) -C userland always BUILD_DIR=$(abspath $(BUILD_DIR))

clean:
	$(MAKE) -C userproc clean BUILD_DIR=$(abspath $(BUILD_DIR))
	$(MAKE) -C userland clean BUILD_DIR=$(abspath $(BUILD_DIR))
	rm -rf $(BUILD_DIR)

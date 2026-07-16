USER_TARGET_ASMFLAGS +=
USER_TARGET_CFLAGS +=
USER_TARGET_LINKFLAGS +=
USER_TARGET_LIBS +=

.PHONY: all clean always userproc

all: userproc

userproc:
	@$(MAKE) -C userproc BUILD_DIR=$(abspath $(BUILD_DIR))

always:
	$(MAKE) -C userproc always BUILD_DIR=$(abspath $(BUILD_DIR))
	$(MAKE) -C userland always BUILD_DIR=$(abspath $(BUILD_DIR))

clean:
	$(MAKE) -C userproc clean BUILD_DIR=$(abspath $(BUILD_DIR))
	rm -rf $(BUILD_DIR)

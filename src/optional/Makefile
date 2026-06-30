features =

ifeq ($(Include_BSW), 1)
features += BSW
endif

all:
	@$(foreach module, $(features), @$(MAKE) -C $(module) BUILD_DIR=$(abspath $(BUILD_DIR)))
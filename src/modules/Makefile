
MODULES := $(shell find ./** -name "makefile")
MODULES_SRC_DIR := $(notdir $(subst /makefile, , $(MODULES)))

MODULE_NAME = 

all:
	@$(foreach module, $(MODULES_SRC_DIR), @$(MAKE) -C $(module) BUILD_DIR=$(abspath $(BUILD_DIR)) MODULE_NAME=$(module))

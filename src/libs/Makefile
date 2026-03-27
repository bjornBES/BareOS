TARGET64_ASMFLAGS += -f elf64 -I.
TARGET64_CFLAGS += -ffreestanding -nostdlib -I. -I $(INCLUDE_DIR)
TARGET64_CXXFLAGS += -ffreestanding -nostdlib -I. -I $(INCLUDE_DIR)
TARGET64_LIBS += -lgcc
TARGET64_LINKFLAGS += -nostdlib


HEADERS_C = $(shell find . -type f -name "*.h" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.h")

SOURCES_C = $(shell find . -type f -name "*.c" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.c")

OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/libcore/c/%.obj, $(SOURCES_C))

HEADERS_CXX = $(shell find . -type f -name "*.hpp" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.hpp")

SOURCES_CXX = $(shell find . -type f -name "*.cpp" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.cpp")

OBJECTS_CXX = $(patsubst %.cpp, $(BUILD_DIR)/libcore/cpp/%.obj, $(SOURCES_CXX))

HEADERS_ASM = $(shell find . -type f -name "*.inc" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.inc")

SOURCES_ASM = $(shell find . -type f -name "*.asm" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/libcore/asm/%.obj, $(SOURCES_ASM))

libcore: $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libcore.a: $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_CXX)
	@$(TARGET64_AR) rcs $@ $^
	@echo "--> Created:  libcore.a"

$(BUILD_DIR)/libcore/cpp/%.obj: %.cpp $(HEADERS_CXX)
	@mkdir -p $(@D)
	@$(TARGET64_CXX) $(TARGET64_CXXFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libcore/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET64_CC) $(TARGET64_CFLAGS) -c -o $@ $<
	@$(TARGET64_CC) $(TARGET64_CFLAGS) -c -save-temps -o $@.c $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libcore/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET64_ASM) $(TARGET64_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

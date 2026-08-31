TARGET_ASFLAGS +=
TARGET_CFLAGS +=
TARGET_LDFLAGS +=

BUILD_DIR_LIBCORE = $(BUILD_DIR)/libcore
BUILD_DIR_C = $(BUILD_DIR_LIBCORE)/c
BUILD_DIR_ASM = $(BUILD_DIR_LIBCORE)/asm
BUILD_DIR_CBL = $(BUILD_DIR_LIBCORE)/cobol
TEMP_DIR_CBL = $(BUILD_DIR_LIBCORE)/cobol/temp

HEADERS_C = 	$(shell find . -type f -name "*.h" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.h")

SOURCES_C = 	$(shell find . -type f -name "*.c" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.c")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/libcore/c/%.obj, $(SOURCES_C))

HEADERS_CXX = 	$(shell find . -type f -name "*.hpp" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.hpp")

SOURCES_CXX = 	$(shell find . -type f -name "*.cpp" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.cpp")

OBJECTS_CXX = 	$(patsubst %.cpp, $(BUILD_DIR)/libcore/cpp/%.obj, $(SOURCES_CXX))

HEADERS_ASM = 	$(shell find . -type f -name "*.inc" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.inc")

SOURCES_ASM = 	$(shell find . -type f -name "*.asm" -not -path "./arch/*") \
				$(shell find ${SOURCE_ARCH} -type f -name "*.asm")

OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/libcore/asm/%.obj, $(SOURCES_ASM))


OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR_ASM)/%.obj, $(SOURCES_ASM))


.PHONY: all libcore clean always

all: libcore

libcore: $(BUILD_DIR)/libcore.a

$(BUILD_DIR)/libcore.a: $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_CXX)
	@$(TARGET_AR) rcs $@ $^
	@echo "$(AR_PREFIX) libcore.a"

$(BUILD_DIR_CXX)/%.obj: %.cpp $(HEADERS_CXX)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(TARGET_CXXFLAGS) -c -o $@ $<
	@echo "$(CX_PREFIX)" $<

$(BUILD_DIR_C)/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) -m64 $(TARGET_CFLAGS) -c -o $@ $<
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

clean:
	@rm -f $(BUILD_DIR)/libcore.a

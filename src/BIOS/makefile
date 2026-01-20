TARGET_ASMFLAGS += -f elf -I.
TARGET_CFLAGS += -ffreestanding -nostdlib -I. -I $(INCLUDE_DIR)
TARGET_CXXFLAGS += -ffreestanding -nostdlib -I. -I $(INCLUDE_DIR)

HEADERS_C = $(shell find $(arch)/. -type f -name "*.h")

SOURCES_C = $(shell find $(arch)/. -type f -name "*.c") \

OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/libbios/c/%.obj, $(SOURCES_C))

HEADERS_CXX = $(shell find $(arch)/. -type f -name "*.hpp")

SOURCES_CXX = $(shell find $(arch)/. -type f -name "*.cpp") \

OBJECTS_CXX = $(patsubst %.cpp, $(BUILD_DIR)/libbios/cpp/%.obj, $(SOURCES_CXX))

HEADERS_ASM = $(shell find $(arch)/. -type f -name "*.inc")

SOURCES_ASM = $(shell find $(arch)/. -type f -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/libbios/asm/%.obj, $(SOURCES_ASM))

libbios: $(BUILD_DIR)/libbios.a

$(BUILD_DIR)/libbios.a: $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_CXX)
	@$(TARGET_AR) rcs $@ $^
#s	@$(TARGET_LD) $(TARGET_LINKFLAGS) -shared -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created:  libbios.a"

$(BUILD_DIR)/libbios/cpp/%.obj: %.cpp $(HEADERS_CXX)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(TARGET_CXXFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libbios/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libbios/asm/%.obj: %.asm
	mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -rf $(BUILD_DIR)

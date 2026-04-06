TOOLCHAIN_PREFIX    = $(abspath ./toolchain/$(TARGET))
TOOLCHAIN_PREFIX32  = $(abspath ./toolchain/i686-elf)
export PATH := $(TOOLCHAIN_PREFIX)/bin:$(TOOLCHAIN_PREFIX32)/bin:$(PATH)

toolchain: toolchain_binutils toolchain_gcc toolchain_binutils32 toolchain_gcc32

# --- x86_64-elf (existing) ---
BINUTILS_SRC   = toolchain/binutils-$(BINUTILS_VERSION)
BINUTILS_BUILD = toolchain/binutils-build-$(BINUTILS_VERSION)
GCC_SRC        = toolchain/gcc-$(GCC_VERSION)
GCC_BUILD      = toolchain/gcc-build-$(GCC_VERSION)

toolchain_binutils: $(TOOLCHAIN_PREFIX)/bin/$(TARGET)-ld
$(TOOLCHAIN_PREFIX)/bin/$(TARGET)-ld: $(BINUTILS_SRC).tar.xz $(BINUTILS_SRC)
	mkdir -p $(BINUTILS_BUILD)
	cd $(BINUTILS_BUILD) && CFLAGS= ASMFLAGS= CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld ASM= LINKFLAGS= TARGET= arch= config= LIBS= \
	../binutils-$(BINUTILS_VERSION)/configure \
	    --prefix="$(TOOLCHAIN_PREFIX)" \
	    --target=$(TARGET)             \
	    --with-sysroot                 \
	    --disable-nls                  \
	    --disable-werror
	CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld CFLAGS= ASMFLAGS= LIBS= LINKFLAGS= ASM= \
	$(MAKE) -C $(BINUTILS_BUILD)
	$(MAKE) -C $(BINUTILS_BUILD) install

toolchain_gcc: $(TOOLCHAIN_PREFIX)/bin/$(TARGET)-gcc
$(TOOLCHAIN_PREFIX)/bin/$(TARGET)-gcc: $(TOOLCHAIN_PREFIX)/bin/$(TARGET)-ld $(GCC_SRC).tar.xz
	cd toolchain && tar -xf gcc-$(GCC_VERSION).tar.xz
	mkdir -p $(GCC_BUILD)
	cd $(GCC_BUILD) && CFLAGS= ASMFLAGS= CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld ASM= LINKFLAGS= TARGET= arch= config= LIBS= \
	../gcc-$(GCC_VERSION)/configure \
	    --prefix="$(TOOLCHAIN_PREFIX)" \
	    --target=$(TARGET)             \
	    --disable-nls                  \
	    --enable-languages=c,c++       \
	    --without-headers
	CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld CFLAGS= ASMFLAGS= LIBS= LINKFLAGS= ASM= \
	$(MAKE) -j8 -C $(GCC_BUILD) all-gcc all-target-libgcc
	$(MAKE) -C $(GCC_BUILD) install-gcc install-target-libgcc

# --- i686-elf (new) ---
BINUTILS_BUILD32 = toolchain/binutils-build32-$(BINUTILS_VERSION)
GCC_BUILD32      = toolchain/gcc-build32-$(GCC_VERSION)

toolchain_binutils32: $(TOOLCHAIN_PREFIX32)/bin/i686-elf-ld
$(TOOLCHAIN_PREFIX32)/bin/i686-elf-ld: $(BINUTILS_SRC).tar.xz $(BINUTILS_SRC)
	mkdir -p $(BINUTILS_BUILD32)
	cd $(BINUTILS_BUILD32) && CFLAGS= ASMFLAGS= CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld ASM= LINKFLAGS= TARGET= arch= config= LIBS= \
	../binutils-$(BINUTILS_VERSION)/configure \
	    --prefix="$(TOOLCHAIN_PREFIX32)" \
	    --target=i686-elf                \
	    --with-sysroot                   \
	    --disable-nls                    \
	    --disable-werror
	CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld CFLAGS= ASMFLAGS= LIBS= LINKFLAGS= ASM= \
	$(MAKE) -C $(BINUTILS_BUILD32)
	$(MAKE) -C $(BINUTILS_BUILD32) install

toolchain_gcc32: $(TOOLCHAIN_PREFIX32)/bin/i686-elf-gcc
$(TOOLCHAIN_PREFIX32)/bin/i686-elf-gcc: $(TOOLCHAIN_PREFIX32)/bin/i686-elf-ld $(GCC_SRC).tar.xz
	mkdir -p $(GCC_BUILD32)
	cd $(GCC_BUILD32) && CFLAGS= ASMFLAGS= CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld ASM= LINKFLAGS= TARGET= arch= config= LIBS= \
	../gcc-$(GCC_VERSION)/configure \
	    --prefix="$(TOOLCHAIN_PREFIX32)" \
	    --target=i686-elf               \
	    --disable-nls                   \
	    --enable-languages=c,c++        \
	    --without-headers
	CC=/usr/bin/gcc CXX=/usr/bin/g++ LD=/usr/bin/ld CFLAGS= ASMFLAGS= LIBS= LINKFLAGS= ASM= \
	$(MAKE) -j8 -C $(GCC_BUILD32) all-gcc all-target-libgcc
	$(MAKE) -C $(GCC_BUILD32) install-gcc install-target-libgcc

# --- Downloads (shared) ---
$(BINUTILS_SRC):
	cd toolchain && tar -xf binutils-$(BINUTILS_VERSION).tar.xz
$(BINUTILS_SRC).tar.xz:
	mkdir -p toolchain
	cd toolchain && wget $(BINUTILS_URL)
$(GCC_SRC).tar.xz:
	mkdir -p toolchain
	cd toolchain && wget $(GCC_URL)

# --- Clean ---
clean-toolchain:
	rm -rf $(GCC_BUILD) $(GCC_SRC) $(BINUTILS_BUILD) $(BINUTILS_SRC) \
	       $(GCC_BUILD32) $(BINUTILS_BUILD32)
clean-toolchain-all:
	rm -rf toolchain/*

.PHONY: toolchain toolchain_binutils toolchain_gcc \
        toolchain_binutils32 toolchain_gcc32 \
        clean-toolchain clean-toolchain-all
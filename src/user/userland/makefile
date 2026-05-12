
SYSROOT_DIR +=

MUSL 				= musl-1.2.5
MUSL_SRC   			= musl/$(MUSL)
MUSL_BUILD   		= musl

BUSYBOX_SRC        	= busybox 
BUSYBOX_BUILD       = busybox

ROOTFS_EXTERNAL = $(SYSROOT_DIR)/include $(SYSROOT_DIR)/lib

.PHONY: all clean always musl_make busybox_make

all: busybox_make musl_make

menuconfig-busybox:
	$(MAKE) -C $(BUSYBOX_SRC) menuconfig

install:
	mkdir -p $(SYSROOT_DIR)
	CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= TARGET= arch= config= LIBS= \
	$(MAKE) -j16 -C $(BUSYBOX_SRC) \
		CONFIG_SYSROOT=$(SYSROOT_DIR) CONFIG_PREFIX=$(SYSROOT_DIR) \
		CROSS_COMPILE=$(CROSS) \
		CFLAGS="--sysroot=$(SYSROOT_DIR)/include -nostdlib -ffreestanding -I$(SYSROOT_DIR)/include" \
		LDFLAGS="--sysroot=$(SYSROOT_DIR)/include -static -I$(SYSROOT_DIR)/include" \
		install

musl_make: $(MUSL_SRC) rootfs_external
	@mkdir -p $(MUSL_SRC)
	mkdir -p $(SYSROOT_DIR)
	@cd $(MUSL_SRC) && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= TARGET= arch= config= LIBS= \
	./configure \
	  --prefix=$(SYSROOT_DIR) \
	  --target=x86_64-elf \
	  --syslibdir=$(SYSROOT_DIR)/lib \
	  CC=x86_64-elf-gcc \
	  CROSS_COMPILE=x86_64-elf-
	
	@echo musl make
	$(MAKE) -j8 -C $(MUSL_SRC)
	@echo musl install
	$(MAKE) -j8 -C $(MUSL_SRC) install

$(MUSL_SRC): $(MUSL_SRC).tar.gz
	cd musl && tar -xf $(MUSL).tar.gz
	
$(MUSL_SRC).tar.gz:
	@mkdir -p musl
	@cd musl && wget https://musl.libc.org/releases/musl-1.2.5.tar.gz

rootfs_external: $(SYSROOT_DIR)/include $(SYSROOT_DIR)/lib

$(SYSROOT_DIR)/include:
	mkdir -p $(SYSROOT_DIR)/include
$(SYSROOT_DIR)/lib:
	mkdir -p $(SYSROOT_DIR)/lib

busybox_make: $(BUSYBOX_SRC)/.config musl_make
	@mkdir -p $(BUSYBOX_SRC)
	@mkdir -p $(SYSROOT_DIR)/bin

	CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= TARGET= arch= config= LIBS= \
	$(MAKE) -j16 -C $(BUSYBOX_SRC) \
		CROSS_COMPILE=$(CROSS) \
		CFLAGS="--sysroot=$(SYSROOT_DIR)/include -nostdlib -ffreestanding -I$(SYSROOT_DIR)/include" \
		LDFLAGS="--sysroot=$(SYSROOT_DIR)/include -static -I$(SYSROOT_DIR)/include" \
		install

$(BUSYBOX_SRC)/.config: $(BUSYBOX_SRC)
	$(MAKE) -C $(BUSYBOX_SRC) CROSS_COMPILE=$(CROSS) allnoconfig

$(BUSYBOX_SRC):
	git clone git://busybox.net/busybox.git	busybox

always:

clean:
	rm busybox -dir -r -f
	rm musl -dir -r -f
	rm $(SYSROOT_DIR)/include -r -dir -f
	rm $(SYSROOT_DIR)/lib -r -dir -f
	rm $(SYSROOT_DIR)/bin -r -dir -f

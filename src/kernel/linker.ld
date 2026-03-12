ENTRY(entry)
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
KERNEL_VMA = 0xC0000000;
KERNEL_LMA = 0x00100000;

SECTIONS
{
    . = KERNEL_LMA;

	_kernel_start = .;

    . = KERNEL_VMA;

    .text               : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA)
    {
        __text_start = .;       *(.text)
    }

    .data               : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA)
    {
        __data_start = .;       *(.data)
    }

    .rodata             : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        __rodata_start = .;     *(.rodata)
    }

    .bss                : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA)
    { 
        __bss_start = .;        *(.bss)     
    }
    
    /* Kernel Stack */
    . = ALIGN(4096);
    stack_bottom = .;
    . += 16384;       /* 16 KB */
    stack_top = .;
    . = ALIGN(4096);

    /* VGA buffer */
    . = ALIGN(4096);
    VGAModesAddr = .;
    . += 8192;

    /* VGA Configurations */
    . = ALIGN(4096);
    VGAConfigurations = .;
    . += 8192;

    kernel_end = .;

    /* End of kernel */
    . = ALIGN(4096);
    __end = .;

}

ENTRY(entry)
OUTPUT_FORMAT("elf32-i386")
KERNEL_VMA = 0xC0000000;
KERNEL_LMA = 0x00100000;

SECTIONS
{
    . = KERNEL_LMA;
    .text               : /* AT(KERNEL_LMA) */
    {
        __text_start = .;       *(.text)
    }

    .data               : /* AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA) */
    {
        __data_start = .;       *(.data)
    }

    .rodata             : /* AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA) */
    {
        __rodata_start = .;     *(.rodata)
    }
    
    .font               : /* AT(ADDR(.font) - KERNEL_VMA + KERNEL_LMA) */
    {
        __font_start = .;       *(.font)
    }

    .bss                :
    { 
        __bss_start = .;        *(.bss)     
    }

    /* Kernel Stack */
    . = ALIGN(4096);
    stack_bottom = .;
    . += 16384;       /* 16 KB */
    stack_top = .;
    . = ALIGN(4096);
    
    
    /* Boot parameters */
    . = ALIGN(4096);
    bootParamsAddr = .;
    . += 16384;

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
    __end = .;
    . = ALIGN(4096);

}

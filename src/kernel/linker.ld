ENTRY(entry)
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
KERNEL_VMA = 0xC0000000;
KERNEL_LMA = 0x00100000;

FRAME_BITMAP_SIZE = (1024 * 1024) / 8; /* // each bit is a 4KB frame */

SECTIONS
{
    . = KERNEL_LMA;

    . = KERNEL_VMA;

	__kernel_start = .;

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

    . = ALIGN(4096);
    __kernel_end = .;
    __kernel_end_phys = . - KERNEL_VMA + KERNEL_LMA;

    __frame_bitmap = .;
    __frame_bitmap_phys = . - KERNEL_VMA + KERNEL_LMA;
    . += FRAME_BITMAP_SIZE;
    __frame_bitmap_phys_end = . - KERNEL_VMA + KERNEL_LMA;
    __frame_bitmap_size = FRAME_BITMAP_SIZE;

    /* End of kernel */
    . = ALIGN(4096);
    __end = .;
    __heap_size = 1024 * 256;
}

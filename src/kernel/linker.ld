ENTRY(entry)
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
KERNEL_VMA = 0xFFFFFFFF80000000;
KERNEL_LMA = 0x0000000000100000;

/* yes need to update now */
FRAME_BITMAP_SIZE = (1024 * 1024) / 8;

PHDRS
{
    text PT_LOAD FLAGS(5);   /* R+X */
    data PT_LOAD FLAGS(6);   /* R+W */
}

SECTIONS
{
    __KERNEL_VIRT = KERNEL_VMA;
    __KERNEL_PHYS = KERNEL_LMA;
    . = KERNEL_VMA;

	__kernel_start = .;

    .text               : AT(KERNEL_LMA)
    {
        __text_start = .;
        *(.text .text*)
    } :text

    .rodata             : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        __rodata_start = .;
        *(.rodata .rodata*)
    } :text

    . = ALIGN(4096);

    .data               : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA)
    {
        __data_start = .;
        *(.data .data*)
    } :data

    .bss                : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        __bss_start = .;
        *(.bss .bss*)
        *(COMMON)
    } :data
    
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

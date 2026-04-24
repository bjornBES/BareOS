ENTRY(entry)

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

    __kernel_pages = . - KERNEL_VMA + KERNEL_LMA;
    . += 0x400000;  /* 4 MiB */

    __frame_bitmap = .;
    __frame_bitmap_phys = . - KERNEL_VMA + KERNEL_LMA;
    . += FRAME_BITMAP_SIZE;
    __frame_bitmap_phys_end = . - KERNEL_VMA + KERNEL_LMA;
    __frame_bitmap_size = FRAME_BITMAP_SIZE;

    /* End of kernel */
    . = ALIGN(0x10000);
    __kernel_end = .;
    __kernel_end_phys = . - KERNEL_VMA + KERNEL_LMA;
    __heap_size = HEAP_SIZE;
}

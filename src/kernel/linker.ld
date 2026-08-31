ENTRY(asm_entry)

PHDRS
{
    init PT_LOAD FLAGS(5);   /* R+X */
    text PT_LOAD FLAGS(5);   /* R+X */
    data PT_LOAD FLAGS(6);   /* R+W */
    vdso PT_LOAD FLAGS(5);   /* R+X */
}

SECTIONS
{
    . = KERNEL_VMA;

    .init.text              : AT(KERNEL_LMA)
    {
        __init_text_start = .;
        *(.init.text .init.text*)
    } :init
    /*
    .init.rodata            : AT(ADDR(.init.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        __init_rodata_start = .;
        *(.init.rodata .init.rodata*)
    } :init

    .init.data              : AT(ADDR(.init.data) - KERNEL_VMA + KERNEL_LMA)
    {
        __init_data_start = .;
        *(.init.data .init.data*)
    } :data
    .init.bss               : AT(ADDR(.init.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        __init_bss_start = .;
        *(.init.bss .init.bss*)
    } :data
    */

    .text                   : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA)
    {
        __text_start = .;
        *(.text .text*)
    } :text

    .rodata                 : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        __rodata_start = .;
        *(.rodata .rodata*)
    } :text

    . = ALIGN(4096);

    .data                   : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA)
    {
        __data_start = .;
        *(.data)
        *(.data.*)
    } :data

    .bss                    : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        __bss_start = .;
        *(.bss .bss*)
        *(COMMON)
    } :data

    . = ALIGN(0x1000);
    .vdso                   : AT(ADDR(.vdso) - KERNEL_VMA + KERNEL_LMA)
    {
        __vdso_start = .;
        *(.vdso)
    } :vdso
    . = ALIGN(0x1000);
    __vdso_end = .;
    
    /* Kernel Stack */
    . = ALIGN(4096);
    stack_bottom = .;
    . += 4096;       /* 16 KB */
    stack_top = .;
    . = ALIGN(4096);

    __frame_bitmap = .;
    __frame_bitmap_phys = . - KERNEL_VMA + KERNEL_LMA;
    . += FRAME_BITMAP_SIZE;
    __frame_bitmap_phys_end = . - KERNEL_VMA + KERNEL_LMA;
    __frame_bitmap_size = FRAME_BITMAP_SIZE;

    /* End of kernel */
    . = ALIGN(0x10000);
    /*
    __kernel_end = .;
    __kernel_end_phys = . - KERNEL_VMA + KERNEL_LMA;
    */
    __heap_size = HEAP_SIZE;
}

ENTRY(asm_entry)

PHDRS
{
    init        PT_LOAD FLAGS(5);   /* R+X */
    init_data   PT_LOAD FLAGS(6);   /* R+W */
    text        PT_LOAD FLAGS(5);   /* R+X */
    data        PT_LOAD FLAGS(6);   /* R+W */
    vdso        PT_LOAD FLAGS(5);   /* R+X */
}

SECTIONS
{
    pmm_early_heap_size = EARLY_HEAP_SIZE;

    . = KERNEL_VMA;

    .init.text              : AT(KERNEL_LMA)
    {
        __init_text_start = .;
        *(.init.text .init.text*)
    } :init

    .init.rodata            : AT(ADDR(.init.rodata) - KERNEL_VMA + KERNEL_LMA)
    {
        __init_rodata_start = .;
        *(.init.rodata .init.rodata*)
    } :init

    .init.data              : AT(ADDR(.init.data) - KERNEL_VMA + KERNEL_LMA)
    {
        __init_data_start = .;
        *(.init.data .init.data*)
    } :init_data
    .init.bss               : AT(ADDR(.init.bss) - KERNEL_VMA + KERNEL_LMA)
    {
        __init_bss_start = .;
        *(.init.bss .init.bss*)
    } :init_data

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
}

ENTRY(_start)
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

PHDRS {
    text PT_LOAD FLAGS(5);   /* r-x */
    data PT_LOAD FLAGS(6);   /* rw- */
}

SECTIONS {
    . = 0x00008000;

    .text   : { *(.text*)   } :text
    .rodata : { *(.rodata*) } :text
    .data   : { *(.data*)   } :data
    .bss    : { *(.bss*) *(COMMON) } :data

    /DISCARD/ : { *(.comment) *(.note*) }
}
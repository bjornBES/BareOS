ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS {
    . = 0x00008000;     /* user space load address */

    .text   : { *(.text*)   }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*)   }
    .bss    : { *(.bss*)  *(COMMON) }

    /DISCARD/ : { *(.comment) *(.note*) }
}
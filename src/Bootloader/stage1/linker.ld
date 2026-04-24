ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x0600;                  /* BIOS (or MBR) puts us at 0x7C00 */
SECTIONS
{
    . = phys;

    /* Code */
    __entry_start = .;
    __vbr_start = 0x7E00;
    .text               : { *(.text)            }

    /* Modifiable data */
    .data               : { *(.data)            }

    /* Read-only data */
    .rodata             : { *(.rodata)          }

    __code_end = .;
    . = 0x1B8 + phys;
    .UDI                : { *(.UDI)       }
    . = 0x1BE + phys;
    .partition          : { __partition = .;
                            *(.partition)
                          }

    /* Required for the BIOS to boot our OS */
    .bios_footer 0x0600+510 : { SHORT(0xAA55)       }

    /* Uninitialized memory (declared with resb/resw/resq). We can use memory
       beyond this binary for that. 
    */
    .bss                : { *(.bss)             }
}

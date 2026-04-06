ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x7C00;                  /* BIOS (or MBR) puts us at 0x7C00 */

SECTIONS
{
    . = phys;

    /* Entry section, which shouldn't be too far down the file (the near jump in
       .fsjump is limited to 127 bytes)
    */
    .entry              : { __entry_start = .;
                            *(.entry)   
                          }

    /* Code */
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
    .bios_footer 0x7DFE : { SHORT(0xAA55)       }

    /* Uninitialized memory (declared with resb/resw/resq). We can use memory
       beyond this binary for that. 
    */
    .bss                : { *(.bss)             }
}

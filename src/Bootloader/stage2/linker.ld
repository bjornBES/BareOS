ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x00008000;

SECTIONS
{
    /*
    . = 0x00000500;
    .bios               : { __bios_start = .;       *(.bios)    }
    */

    . = phys;

    .entry              : { __entry_start = .;      *(.entry)   }
    .text               : { __text_start = .;       *(.text)    }
    .data               : { __data_start = .;       *(.data)    }
    .rodata             : { __rodata_start = .;     *(.rodata)  }
    .bss                : { __bss_start = .;        *(.bss)     }
    
    __end = .;
}

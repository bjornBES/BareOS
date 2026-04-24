ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x00008000;

SECTIONS
{
    . = phys;
    __stack_top = .;
    .entry              : { __entry_start = .;      *(.entry)   }
    .text               : { __text_start = .;       *(.text)    }
    .data               : { __data_start = .;       *(.data)    }
    .rodata             : { __rodata_start = .;     *(.rodata)  }
    .bss                : { __bss_start = .;        *(.bss)     }
    . = ALIGN(4096);
    __fat_driver = .;

    res = 0x00080000;
    END = 0x000FFFFF;

    kernel_phys_load = 0x00200000;

    __end = .;
}


ENTRY(stage2_entry)
phys = 0x00008000;

SECTIONS
{
    . = phys;
    __stack_top = .;

    .entry              : AT(phys) { __entry_start = .;      *(.entry)   }
    . = ALIGN(4096);
    /* fuck that is low */
    __trampoline_start = .;
    .smp_trampoline_text :
    {
        *(.smp_trampoline_text)
    }
    . = ALIGN(4096);
    __trampoline_end = .;

    .arch_text          : { __arch_text_start = .;  *(.arch_text)}
    .text               : { __text_start = .;       *(.text)    }
    .data               : { __data_start = .;       *(.data)    }
    .rodata             : { __rodata_start = .;     *(.rodata)  }
    .bss                : { __bss_start = .;        *(.bss)     }
    text_end = .;
    
    __fat_driver = 0x00040000;

    res = 0x00080000;
    END = 0x000FFFFF;

    kernel_phys_load = 0x00200000;

    __end = .;
}

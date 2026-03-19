    target remote :1234
    symbol-file /mnt/EDrive/projects/BareOS/build/i686_debug/kernel/kernel.elf
    set disassembly-flavor intel
    b main

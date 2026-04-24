target remote :1234
symbol-file /mnt/EDrive/projects/BareOS/build/x86_64_debug/kernel/kernel.elf
set disassembly-flavor intel
b main
b panic

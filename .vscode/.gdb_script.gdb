target remote :1234
symbol-file /mnt/EDrive/projects/BareOS/build/x86_64_debug/kernel/kernel.elf
add-symbol-file /mnt/EDrive/projects/BareOS/build/x86_64_debug/stage2/stage2.elf 0x8000
add-symbol-file /mnt/EDrive/projects/BareOS/build/x86_64_debug/user/init.elf 0x1000
add-symbol-file /mnt/EDrive/projects/BareOS/rootfs/user/bin/ash 0x400000
set disassembly-flavor intel
b kernel_main
b panic

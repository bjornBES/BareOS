target remote :1234
symbol-file /home/BjornBEsV2/projects/BareOS/build/x86_64_debug/kernel/kernel.elf
set disassembly-flavor intel
b main
b panic

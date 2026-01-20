    symbol-file /mnt/f99d9233-1253-481c-b2b4-9a3f0c96fc12/projects/newOs/build/i686_debug/kernel/kernel.elf
    set disassembly-flavor intel
    target remote | qemu-system-i386 -S -gdb stdio -m 8g -d guest_errors -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0 -drive file=/mnt/f99d9233-1253-481c-b2b4-9a3f0c96fc12/projects/newOs/build/i686_debug/image.img,format=raw,id=disk,if=ide -device intel-hda -device sb16

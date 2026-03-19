#!/bin/bash

QEMU_ARGS="-S -debugcon stdio -gdb tcp::1234 -m 4g -d guest_errors -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0"

if [ "$#" -le 1 ]; then
    echo "Usage: ./debug.sh <image_type> <image> <floppy_image>"
    exit 1
fi

clear

IMAGE=$2

if [ "$1" == "floppy" ]; then
    QEMU_ARGS="${QEMU_ARGS} -fda $IMAGE"
elif [ "$1" == "disk" ]; then
    QEMU_ARGS="${QEMU_ARGS} -drive file=$IMAGE,format=raw,id=disk,if=none -device ahci,id=ahci -device ide-hd,drive=disk"
else
    echo "Unknown image type: $1"
    exit 2
fi

QEMU_ARGS="${QEMU_ARGS} -device intel-hda"
QEMU_ARGS="${QEMU_ARGS} -device sb16"

if command -v gnome-terminal &>/dev/null; then
    gnome-terminal -- qemu-system-i386 $QEMU_ARGS
elif command -v konsole &>/dev/null; then
    konsole -e bash -c "qemu-system-i386 $QEMU_ARGS"&
elif command -v xterm &>/dev/null; then
    xterm -e bash -c "qemu-system-i386 $QEMU_ARGS"
else
    echo "No supported terminal emulator found"
    exit 1
fi

cat > .vscode/.gdb_script.gdb << EOF
    target remote :1234
    symbol-file $PWD/build/i686_debug/kernel/kernel.elf
    set disassembly-flavor intel
    b main
EOF

gf2-gdb $PWD/build/i686_debug/kernel/kernel.elf -x .vscode/.gdb_script.gdb

#!/bin/bash

QEMU_ARGS="-S -debugcon stdio -gdb tcp::1234 -m 4g -d guest_errors -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0"

if [ "$#" -le 1 ]; then
    echo "Usage: ./debug.sh <image_type> <arch> <image> <kernel>"
    exit 1
fi

clear

IMAGE=$3

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

QEMU_COMMAND=qemu-system
if [ "$2" == "i686" ]; then
    QEMU_COMMAND=${QEMU_COMMAND}-i386
else
    QEMU_COMMAND=${QEMU_COMMAND}-$2
fi

if command -v gnome-terminal &>/dev/null; then
    gnome-terminal -- ${QEMU_COMMAND} ${QEMU_ARGS}
elif command -v konsole &>/dev/null; then
    konsole -e bash -c "${QEMU_COMMAND} ${QEMU_ARGS}"&
elif command -v xterm &>/dev/null; then
    xterm -e bash -c "${QEMU_COMMAND} ${QEMU_ARGS}"
else
    echo "No supported terminal emulator found"
    exit 1
fi

echo "running qemu ${QEMU_COMMAND}"

KERNEL=$4

cat > .vscode/.gdb_script.gdb << EOF
target remote :1234
symbol-file $KERNEL
set disassembly-flavor intel
b main
b panic
EOF

if command -v gf2-gdb &>/dev/null; then
    gf2-gdb $KERNEL -x .vscode/.gdb_script.gdb
#    gdb $KERNEL -x .vscode/.gdb_script.gdb
else
    gdb $KERNEL -x .vscode/.gdb_script.gdb
fi
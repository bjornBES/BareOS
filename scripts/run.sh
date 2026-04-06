#!/bin/bash

QEMU_ARGS="-k da -debugcon stdio -m 8m -d guest_errors,int,mmu -D debug.txt -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0"

if [ "$#" -le 1 ]; then
    echo "Usage: ./run.sh <image_type> <image>"
    exit 1
fi

clear

IMAGE=$2

if [ "$1" == "floppy" ]; then
    QEMU_ARGS="${QEMU_ARGS} -fda $IMAGE"
    QEMU_ARGS="${QEMU_ARGS} -drive format=raw,id=disk,if=none"
elif [ "$1" == "disk" ]; then
    QEMU_ARGS="${QEMU_ARGS} -drive file=$IMAGE,format=raw,id=disk,if=none -device ahci,id=ahci -device ide-hd,drive=disk"

else
    echo "Unknown image type: $1"
    exit 2
fi

QEMU_ARGS="${QEMU_ARGS} -device intel-hda"
QEMU_ARGS="${QEMU_ARGS} -device sb16"
QEMU_ARGS="${QEMU_ARGS} -device vmware-svga"

qemu-system-x86_64 $QEMU_ARGS

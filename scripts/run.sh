#!/bin/bash

QEMU_ARGS="-k da -debugcon stdio -m 1g -d guest_errors,int,invalid_mem -D debug.txt -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0"

if [ "$#" -le 1 ]; then
    echo "Usage: ./run.sh <image_type> <arch> <image>"
    exit 1
fi

clear

IMAGE=$3

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

QEMU_COMMAND=qemu-system
if [ "$2" == "i686" ]; then
    QEMU_COMMAND=${QEMU_COMMAND}-i386
else
    QEMU_COMMAND=${QEMU_COMMAND}-$2
fi

${QEMU_COMMAND} ${QEMU_ARGS}

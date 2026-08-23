#!/bin/bash

QEMU_ARGS="-k da -debugcon stdio -m 1g -d guest_errors,int,invalid_mem,page,strace -D debug.txt -netdev user,id=mynet0 -net nic,model=rtl8139,netdev=mynet0"

if [ "$#" -le 1 ]; then
    echo "Usage: ./run.sh <image_type> <arch> <image>"
    exit 1
fi

clear

IFS=" "
echo "arg = $*"

shift 1
IFS=" "
QEMU_ARGS="$*"
echo "QEMU_ARGS = ${QEMU_ARGS}"

${QEMU_ARGS}

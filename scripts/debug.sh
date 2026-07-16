#!/bin/bash

echo "Usage: ./debug.sh $#"
if [ "$#" -le 2 ]; then
    echo "Usage: ./debug.sh <kernel>"
    exit 1
fi

clear

IFS=" "
echo "$*"

KERNEL=$1

shift 1
IFS=" "
QEMU_ARGS="$*"
echo "${QEMU_ARGS}"

if command -v gnome-terminal &>/dev/null; then
    gnome-terminal -- ${QEMU_ARGS}
elif command -v konsole &>/dev/null; then
    konsole -e bash -c "${QEMU_ARGS}"&
elif command -v xterm &>/dev/null; then
    xterm -e bash -c "${QEMU_ARGS}"
else
    echo "No supported terminal emulator found"
    exit 1
fi

echo "running qemu ${QEMU_ARGS}"

if command -v gf2-gdb &>/dev/null; then
    #gf2-gdb $KERNEL -x .vscode/.gdb_script.gdb
    gdb $KERNEL -x .vscode/.gdb_script.gdb
else
    gdb $KERNEL -x .vscode/.gdb_script.gdb
fi
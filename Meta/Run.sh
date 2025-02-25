#!/bin/bash

QEMU_DEBUG=0
QEMU_SMP_CORES=1
QEMU_MEMORY=512
QEMU_SERIAL=stdio

QEMU_COMMAND=qemu-system-i386

for CMD_ARG in "$@"; do
    case $CMD_ARG in
        --iso-dir=*)
            ISO_DIR=${CMD_ARG#*=}
            if [ ! -d "$ISO_DIR" ]; then
                echo "$$ISO_DIR seems to be un-existing, be sure to build for this target"
                exit 1
            fi
            ;;
        --debug)
            QEMU_DEBUG=1
            ;;
        --smp=*)
            QEMU_SMP_CORES=${CMD_ARG#*=}
            ;;
        --mem=*)
            QEMU_MEMORY=${CMD_ARG#*=}
            ;;
        --serial=*)
            QEMU_SERIAL=${CMD_ARG#*=}
            ;;
        *)
            echo "Run MeetixOS ISO image script"
            echo "USAGE:"
            echo "    Run.sh <--iso-dir=/Path/To/OutputDir> [--debug] [--smp=N] [--mem=N] [--serial=/Path/To/Output]"
            exit 0
            ;;
    esac
done

QEMU_OPTIONS="-m ${QEMU_MEMORY}M -serial $QEMU_SERIAL -cdrom $ISO_DIR/MeetixOS.iso"

if [[ $QEMU_DEBUG -eq 0 ]]; then
    QEMU_OPTIONS="-enable-kvm $QEMU_OPTIONS"
else
    QEMU_OPTIONS="$QEMU_OPTIONS -s -S"
fi

if [[ $QEMU_SMP_CORES -ne 0 ]]; then
    if [[ $QEMU_DEBUG -eq 0 ]]; then
        QEMU_OPTIONS="$QEMU_OPTIONS -cpu host"
    fi

    QEMU_OPTIONS=" $QEMU_OPTIONS -smp $QEMU_SMP_CORES"
fi

# run the OS
$QEMU_COMMAND $QEMU_OPTIONS || exit 1

#!/bin/bash

set -eou pipefail

# Define directories
BUILD_DIR="build"
ISO_DIR="isodir"

# Create necessary directories
mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

# Assemble the bootloader assembly
nasm -f elf boot16.s -o "$BUILD_DIR/boot16.o"
nasm -f elf32 boot32.s -o "$BUILD_DIR/boot32.o"
nasm -f elf32 boot_intel.asm -o "$BUILD_DIR/boot.o"
nasm -f elf add16_wrapper16.s -o "$BUILD_DIR/add16_wrapper16.o"
nasm -f elf32 add16_wrapper32.s -o "$BUILD_DIR/add16_wrapper32.o"

# Compile the kernel
i686-elf-gcc -c kernel.c -o "$BUILD_DIR/kernel.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -c virtual_memory.c -o "$BUILD_DIR/virtual_memory.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -c idt.c -o "$BUILD_DIR/idt.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra

ia16-elf-gcc -c ./add16.c -o "$BUILD_DIR/add16.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra

printf "\nStart of linking\n\n\n"
# Link the kernel and generate the final binary
i686-elf-gcc -T linker_debug.ld -o "$BUILD_DIR/myos.bin" -ffreestanding -O2 -nostdlib \
	"$BUILD_DIR/boot16.o" \
	"$BUILD_DIR/boot32.o" \
	"$BUILD_DIR/boot.o" \
	"$BUILD_DIR/kernel.o" \
	"$BUILD_DIR/virtual_memory.o" \
	"$BUILD_DIR/idt.o" \
	"$BUILD_DIR/add16_wrapper32.o" \
	"$BUILD_DIR/add16_wrapper16.o" \
	"$BUILD_DIR/add16.o" \
	-lgcc

# Start QEMU in debug mode (paused, waiting for GDB)
qemu-system-i386 \
	-kernel "$BUILD_DIR/myos.bin" \
	-s -S \
	-no-reboot \
	-d in_asm,int,cpu_reset \
	-D qemu_instr.log \
	-serial stdio &

QEMU_PID=$!

# Give QEMU a second to start up
sleep 1

# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null

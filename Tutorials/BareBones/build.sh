#!/bin/bash

set -eou pipefail

# Define directories
BUILD_DIR="build"
ISO_DIR="isodir"

# Create necessary directories
mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

# Assemble the bootloader assembly
nasm -f elf32 boot_intel.asm -o "$BUILD_DIR/boot.o"
nasm -f elf32 add16_wrapper32.s -o "$BUILD_DIR/add16_wrapper32.o"
nasm -f elf add16_wrapper16.s -o "$BUILD_DIR/add16_wrapper16.o"

# Link the kernel and generate the final binary
i686-elf-gcc -T linker.ld -o "$BUILD_DIR/myos.bin" -ffreestanding -O2 -nostdlib \
	"$BUILD_DIR/boot.o" \
	"$BUILD_DIR/add16_wrapper16.o" \
	"$BUILD_DIR/add16_wrapper32.o" \
	-lgcc

# Check if the kernel is multiboot-compliant
if grub-file --is-x86-multiboot "$BUILD_DIR/myos.bin"; then
	echo "Multiboot confirmed"
else
	echo "The file is not multiboot"
	exit 1
fi

# Copy the kernel binary and GRUB configuration to the ISO directory
cp "$BUILD_DIR/myos.bin" "$ISO_DIR/boot/myos.bin"
cp grub.cfg "$ISO_DIR/boot/grub/grub.cfg"

# Create the ISO image
grub-mkrescue -o "$BUILD_DIR/myos.iso" "$ISO_DIR"

echo "ISO created successfully: $BUILD_DIR/myos.iso"

qemu-system-i386 -cdrom ./build/myos.iso -d in_asm,int -D qemu_instr.log -no-reboot &
# or do this to use the binary directly
# qemu-system-i386 -kernel ./build/myos.bin &

QEMU_PID=$!

# Give QEMU a second to start up
sleep 1

# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null

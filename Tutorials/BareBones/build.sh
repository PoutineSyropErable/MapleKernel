#!/bin/bash

# Define directories
BUILD_DIR="build"
ISO_DIR="isodir"

# Create necessary directories
mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

# Assemble the bootloader assembly
i686-elf-as boot.s -o "$BUILD_DIR/boot.o"

# Compile the kernel
i686-elf-gcc -c kernel.c -o "$BUILD_DIR/kernel.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# Link the kernel and generate the final binary
i686-elf-gcc -T linker.ld -o "$BUILD_DIR/myos.bin" -ffreestanding -O2 -nostdlib "$BUILD_DIR/boot.o" "$BUILD_DIR/kernel.o" -lgcc

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

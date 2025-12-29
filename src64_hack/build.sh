#!/bin/bash

set -eou pipefail
# Assemble 64-bit boot file

BIT=32

CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra")
# CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra" "-mcmodel=kernel" "-mno-red-zone")
GCC32=i686-elf-gcc
GCC64=x86_64-elf-gcc

if [[ "$BIT" == "32" ]]; then
	GCC="$GCC32"
elif [[ "$BIT" == "64" ]]; then
	GCC="$GCC64"
fi

$GCC "${CFLAGS[@]}" -c kernel.c -o kernel.o -g -O0
nasm -f "elf$BIT" boot_intel.asm -o boot.o -g -F dwarf

# Link the kernel ELF
$GCC -nostdlib -T linker.ld boot.o kernel.o -o kernel.elf -lgcc
objdump -M intel -D kernel.elf >kernel.dump

# Check Multiboot2 header
if grub-file --is-x86-multiboot2 kernel.elf; then
	echo "Multiboot2 confirmed"
else
	echo "The file is not multiboot2"
	exit 1
fi

# Make sure the ISO boot directory exists
mkdir -p iso_root/boot/grub
mkdir -p iso_root/EFI/BOOT

# Copy the kernel into the ISO tree
cp kernel.elf iso_root/boot/

# Build standalone GRUB
grub-mkstandalone \
	-O x86_64-efi \
	--modules="part_gpt part_msdos linux normal configfile" \
	--fonts="" \
	--locales="" \
	--themes="" \
	-o iso_root/EFI/BOOT/BOOTX64.EFI \
	"boot/grub/grub.cfg=./grub_standalone.cfg"

# Create ISO using GRUB
grub-mkrescue -o myos.iso iso_root/

D=()
# D=("-S" "-s")

# Run in QEMU
qemu-system-x86_64 \
	-cdrom myos.iso \
	-no-reboot \
	-smp 4 \
	"${D[@]}" \
	-serial stdio &

QEMU_PID=$!

move_pid_to_workspace $QEMU_PID 21

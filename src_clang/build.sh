#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
ISO_DIR="isodir"

mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra")
NASM_FLAGS32=("-f" "elf32")
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc" "-fno-eliminate-unused-debug-symbols")

nasm "${NASM_FLAGS32[@]}" "boot_intel.asm" -o "$BUILD_DIR/boot.o"
i686-elf-gcc "${CFLAGS[@]}" -c "kernel.c" -o "$BUILD_DIR/kernel.o"
i686-elf-g++ -T linker.ld -o "$BUILD_DIR/myos.elf" "${LDFLAGS[@]}" "$BUILD_DIR/boot.o" "$BUILD_DIR/kernel.o"

USE_MULTIBOOT1=false
if [ "$USE_MULTIBOOT1" == true ]; then
	if grub-file --is-x86-multiboot "$BUILD_DIR/myos.elf"; then
		echo "Multiboot confirmed"
	else
		echo "The file is not multiboot"
		exit 1
	fi
else
	if grub-file --is-x86-multiboot2 "$BUILD_DIR/myos.elf"; then
		echo "Multiboot2 confirmed"
	else
		echo "The file is not multiboot 2"
		exit 1
	fi

fi

# Copy the kernel binary and GRUB configuration to the ISO directory
cp "$BUILD_DIR/myos.elf" "$ISO_DIR/boot/myos.elf"
cp "$ISO_DIR/grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"

# Create the ISO image
grub-mkrescue -o "$BUILD_DIR/myos.iso" "$ISO_DIR"
echo "ISO created successfully: $BUILD_DIR/myos.iso"

QEMU32=qemu-system-i386
QEMU64=qemu-system-x86_64
QEMU="$QEMU64"

# Check if QEMU is already running, kill only if it exists
if pgrep -f "$QEMU" >/dev/null 2>&1; then
	echo "Killing existing QEMU process..."
	pkill -f "$QEMU" 2>/dev/null
	sleep 1 # Give it time to die
fi

$QEMU \
	-cdrom "$BUILD_DIR/myos.iso" \
	\
	-no-reboot \
	-serial stdio & # -vga vmware \

QEMU_PID=$!

sleep 1

# Launch VNC viewer
vncviewer localhost:5900 &
VNC_PID=$!

# sleep 1
if [[ "$MOVE_VNC" == "move" ]]; then
	move_pid_to_workspace $VNC_PID 21
fi

wait $VNC_PID

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null

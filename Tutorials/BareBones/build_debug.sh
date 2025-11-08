#!/bin/bash
# build_debug.sh

set -eou pipefail

# Define directories
BUILD_DIR="build"
ISO_DIR="isodir"

# Create necessary directories
mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

# Assemble the bootloader assembly with debug info
nasm -f elf -g -F dwarf call_realmode_function_wrapper16.s -o "$BUILD_DIR/call_realmode_function_wrapper16.o"
nasm -f elf32 -g -F dwarf call_realmode_function_wrapper32.s -o "$BUILD_DIR/call_realmode_function_wrapper32.o"
nasm -f elf32 -g -F dwarf boot_intel.asm -o "$BUILD_DIR/boot.o"

# Compile the kernel
i686-elf-gcc -c kernel.c -o "$BUILD_DIR/kernel.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
i686-elf-gcc -c virtual_memory.c -o "$BUILD_DIR/virtual_memory.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
i686-elf-gcc -c idt.c -o "$BUILD_DIR/idt.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g

i686-elf-gcc -c string_helper.c -o "$BUILD_DIR/string_helper.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
i686-elf-gcc -c vga_terminal.c -o "$BUILD_DIR/vga_terminal.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
i686-elf-gcc -c pit_timer.c -o "$BUILD_DIR/pit_timer.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g

i686-elf-gcc -c f1_binary_operation.c -o "$BUILD_DIR/f1_binary_operation.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
i686-elf-gcc -c f2_string.c -o "$BUILD_DIR/f2_string.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
i686-elf-gcc -c f3_segment_descriptor_internals.c -o "$BUILD_DIR/f3_segment_descriptor_internals.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g

ia16-elf-gcc -c ./add16.c -o "$BUILD_DIR/add16.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
i686-elf-gcc -c ./push_var_args.c -o "$BUILD_DIR/push_var_args.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -g

printf "\n\n=======Start of linking========\n\n\n"

i686-elf-gcc -T linker.ld -o "$BUILD_DIR/myos.bin" -ffreestanding -O2 -nostdlib \
	"$BUILD_DIR/boot.o" \
	"$BUILD_DIR/kernel.o" \
	"$BUILD_DIR/string_helper.o" \
	"$BUILD_DIR/pit_timer.o" \
	"$BUILD_DIR/vga_terminal.o" \
	"$BUILD_DIR/f1_binary_operation.o" \
	"$BUILD_DIR/f2_string.o" \
	"$BUILD_DIR/f3_segment_descriptor_internals.o" \
	"$BUILD_DIR/virtual_memory.o" \
	"$BUILD_DIR/idt.o" \
	"$BUILD_DIR/call_realmode_function_wrapper32.o" \
	"$BUILD_DIR/call_realmode_function_wrapper16.o" \
	"$BUILD_DIR/add16.o" \
	"$BUILD_DIR/push_var_args.o" \
	-lgcc -g

printf "\n\n=======End of linking========\n\n\n"

# Copy the kernel binary and GRUB configuration to the ISO directory
cp "$BUILD_DIR/myos.bin" "$ISO_DIR/boot/myos.bin"
cp grub.cfg "$ISO_DIR/boot/grub/grub.cfg"

# Create the ISO image
grub-mkrescue -o "$BUILD_DIR/myos.iso" "$ISO_DIR"

echo "ISO created successfully: $BUILD_DIR/myos.iso"

# Create flat binary from ELF (for QEMU boot)

printf "\n\n=======Start of Qemu========\n\n\n"
# Start QEMU in debug mode (paused, waiting for GDB)
qemu-system-i386 \
	-cdrom "$BUILD_DIR/myos.iso" \
	-s -S \
	-no-reboot \
	-d in_asm,int,cpu_reset \
	-D qemu_instr.log \
	-serial stdio &

printf "\n\n=======End of Qemu========\n\n\n"

QEMU_PID=$!

# Give QEMU a second to start up
sleep 1

# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null

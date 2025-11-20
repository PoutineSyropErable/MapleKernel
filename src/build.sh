#!/bin/bash

set -eou pipefail
ARG1="${1:-}"

# Define directories
BUILD_DIR="build"
ISO_DIR="isodir"

CFLAGS=("-std=gnu23" "-ffreestanding" "-Wall" "-Wextra")
CFLAGS16=("-std=gnu99" "-ffreestanding" "-Wall" "-Wextra")
LDFLAGS=("-ffreestanding" "-nostdlib" "-lgcc")
NASM_FLAGS32=("-f" "elf32")
NASM_FLAGS16=("-f" "elf")

DEBUG_OPT_LVL="-O0"
RELEASE_OPT_LVL="-O0"
QEMU_DBG_FLAGS=()

if [[ "$ARG1" == "debug" ]]; then
	echo "Debug mode enabled"
	CFLAGS+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG" "-DQEMU")
	CFLAGS16+=("$DEBUG_OPT_LVL" "-g" "-DDEBUG")
	LDFLAGS+=("-g")
	NASM_FLAGS32+=("-g" "-F" "dwarf")
	NASM_FLAGS16+=("-g" "-F" "dwarf")
	QEMU_DBG_FLAGS+=("-s" "-S")
else
	echo "In normal mode, $RELEASE_OPT_LVL optimisation"
	CFLAGS+=("$RELEASE_OPT_LVL")
	CFLAGS16+=("$RELEASE_OPT_LVL")
	LDFLAGS+=("$RELEASE_OPT_LVL")
fi

# Create necessary directories
mkdir -p "$BUILD_DIR" "$ISO_DIR/boot/grub"

# The variable for the dirs:
KERNEL="kernel"
REAL16_WRAPPERS="real16_wrappers"
REAL_FUNC="realmode_functions"
GDT="gdt"
IDT="idt"
STDIO="stdio"
STDLIB="stdlib"
OTHER="other"
CODE_ANALYSIS="runtime_code_analysis"

# Unused. but moved stuff there eventually
DRIVERS="./drivers"
DRIVERS_PS2="./drivers/ps2"
DRIVERS_PS2_CONTROLLER="./drivers/ps2/controller"
DRIVERS_PS2_KEYBOARD="./drivers/ps2/keyboard"
DRIVERS_PS2_MOUSE="./drivers/ps2/mouse"

DRIVERS_USB_CONTROLLER="./drivers/usb/controller"
ACPI="./acpi"

INCLUDE_DIRS=(
	"$KERNEL"
	"$REAL16_WRAPPERS"
	"$REAL_FUNC"
	"$GDT"
	"$IDT"
	"$STDIO"
	"$STDLIB"
	"$OTHER"
	"$CODE_ANALYSIS"

	"$DRIVERS_PS2_CONTROLLER"
	"$DRIVERS_PS2_KEYBOARD"
	"$DRIVERS_PS2_MOUSE"

	"$ACPI"
	"$DRIVERS_USB_CONTROLLER"
)

# Build SUPER_INCLUDE as an array of -I arguments
SUPER_INCLUDE=()
for dir in "${INCLUDE_DIRS[@]}"; do
	SUPER_INCLUDE+=("-I$dir")
done

# Assemble the bootloader assembly
nasm "${NASM_FLAGS32[@]}" "$KERNEL/boot_intel.asm" -o "$BUILD_DIR/boot.o"
i686-elf-gcc "${CFLAGS[@]}" "${SUPER_INCLUDE[@]}" -c "$KERNEL/kernel.c" -o "$BUILD_DIR/kernel.o"

# Compile the print functions.
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/string_helper.c" -o "$BUILD_DIR/string_helper.o"
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/bit_hex_string.c" -o "$BUILD_DIR/bit_hex_string.o" -std=gnu99 "-I$STDIO" "-I$STDLIB"
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/vga_terminal.c" -o "$BUILD_DIR/vga_terminal.o" "-I$STDIO" "-I$OTHER" "-I$STDLIB"
i686-elf-gcc "${CFLAGS[@]}" -c "$STDIO/stdio.c" -o "$BUILD_DIR/stdio.o" "-I$STDIO" "-I$OTHER" "-I$STDLIB"

#compile misc helper functions
i686-elf-gcc "${CFLAGS[@]}" -c "$STDLIB/stdlib.c" -o "$BUILD_DIR/stdlib.o"

i686-elf-gcc "${CFLAGS[@]}" -c "$GDT/f1_binary_operation.c" -o "$BUILD_DIR/f1_binary_operation.o" "-I$STDIO" "-I$GDT"
i686-elf-gcc "${CFLAGS[@]}" -c "$GDT/f3_segment_descriptor_internals.c" -o "$BUILD_DIR/f3_segment_descriptor_internals.o" "-I$STDIO" "-I$GDT" "-I$STDLIB"
i686-elf-gcc "${CFLAGS[@]}" -c "$GDT/gdt.c" -o "$BUILD_DIR/gdt.o" "-I$STDIO" "-I$GDT" "-I$STDLIB"

# Compile Drivers CPU structures
i686-elf-gcc "${CFLAGS[@]}" -c "$IDT/idt.c" -o "$BUILD_DIR/idt.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO"
i686-elf-gcc "${CFLAGS[@]}" -c "$IDT/pic.c" -o "$BUILD_DIR/pic.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO"
nasm "${NASM_FLAGS32[@]}" "$IDT/exception_handler.asm" -o "$BUILD_DIR/exception_handler.o"

# Compile Drivers
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_CONTROLLER/ps2.c" -o "$BUILD_DIR/ps2.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$ACPI" "-I$DRIVERS_USB_CONTROLLER"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_KEYBOARD/ps2_keyboard.c" -o "$BUILD_DIR/ps2_keyboard.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_MOUSE/ps2_mouse.c" -o "$BUILD_DIR/ps2_mouse.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_KEYBOARD/ps2_keyboard_handler.c" -o "$BUILD_DIR/ps2_keyboard_handler.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_PS2_MOUSE/ps2_mouse_handler.c" -o "$BUILD_DIR/ps2_mouse_handler.o" "-I$IDT" "-I$GDT" "-I$DRIVERS_PS2_CONTROLLER" "-I$STDLIB" "-I$STDIO"

# Temporary stuff. Will properly program them one day.
i686-elf-gcc "${CFLAGS[@]}" -c "$DRIVERS_USB_CONTROLLER/usb_controller.c" -o "$BUILD_DIR/usb_controller.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$DRIVERS_USB_CONTROLLER"
i686-elf-gcc "${CFLAGS[@]}" -c "$ACPI/acpi.c" -o "$BUILD_DIR/acpi.o" "-I$IDT" "-I$GDT" "-I$STDLIB" "-I$STDIO" "-I$ACPI"
i686-elf-gcc "${CFLAGS[@]}" -c "$OTHER/virtual_memory.c" -o "$BUILD_DIR/virtual_memory.o"
i686-elf-gcc "${CFLAGS[@]}" -c "$OTHER/pit_timer.c" -o "$BUILD_DIR/pit_timer.o"

# Just another thing
i686-elf-gcc "${CFLAGS[@]}" -c "$CODE_ANALYSIS/address_getter.c" -o "$BUILD_DIR/address_getter.o" "-I$REAL_FUNC" "-I$REAL16_WRAPPERS" "-I$GDT" "-I$STDLIB"

# Compile the real mode 16 code and it's wrappers
i686-elf-gcc "${CFLAGS[@]}" -c "$REAL16_WRAPPERS/call_real16_wrapper.c" -o "$BUILD_DIR/call_real16_wrapper.o" "-I$STDIO" "-I$STDLIB" "-I$GDT"
nasm "${NASM_FLAGS16[@]}" "$REAL16_WRAPPERS/call_realmode_function_wrapper16.asm" -o "$BUILD_DIR/call_realmode_function_wrapper16.o" "-I$REAL16_WRAPPERS"
nasm "${NASM_FLAGS32[@]}" "$REAL16_WRAPPERS/call_realmode_function_wrapper32.asm" -o "$BUILD_DIR/call_realmode_function_wrapper32.o" "-I$REAL16_WRAPPERS"
ia16-elf-gcc "${CFLAGS16[@]}" -c "$REAL_FUNC/realmode_functions.c" -o "$BUILD_DIR/realmode_functions.o"

# Link the kernel and generate the final binary
printf "\n\n====== Start of Linking =====\n\n"

BUILD_OBJECTS=(
	"$BUILD_DIR/boot.o"
	"$BUILD_DIR/kernel.o"

	"$BUILD_DIR/stdlib.o"

	"$BUILD_DIR/address_getter.o"

	"$BUILD_DIR/string_helper.o"
	"$BUILD_DIR/bit_hex_string.o"
	"$BUILD_DIR/vga_terminal.o"
	"$BUILD_DIR/stdio.o"

	"$BUILD_DIR/exception_handler.o"
	"$BUILD_DIR/idt.o"
	"$BUILD_DIR/pic.o"
	"$BUILD_DIR/ps2.o"
	"$BUILD_DIR/ps2_keyboard.o"
	"$BUILD_DIR/ps2_mouse.o"
	"$BUILD_DIR/ps2_keyboard_handler.o"
	"$BUILD_DIR/ps2_mouse_handler.o"

	"$BUILD_DIR/usb_controller.o"
	"$BUILD_DIR/acpi.o"
	"$BUILD_DIR/virtual_memory.o"
	"$BUILD_DIR/pit_timer.o"

	"$BUILD_DIR/f1_binary_operation.o"
	"$BUILD_DIR/f3_segment_descriptor_internals.o"
	"$BUILD_DIR/gdt.o"

	"$BUILD_DIR/call_realmode_function_wrapper32.o"
	"$BUILD_DIR/call_realmode_function_wrapper16.o"
	"$BUILD_DIR/realmode_functions.o"
	"$BUILD_DIR/call_real16_wrapper.o"
)

i686-elf-gcc -T linker.ld -o "$BUILD_DIR/myos.bin" "${LDFLAGS[@]}" "${BUILD_OBJECTS[@]}"

printf "\n\n====== End of Linking =====\n\n"

# objdump -D -h "$BUILD_DIR/myos.bin" >"$BUILD_DIR/myos.dump"

# Check if the kernel is multiboot-compliant
if grub-file --is-x86-multiboot "$BUILD_DIR/myos.bin"; then
	echo "Multiboot confirmed"
else
	echo "The file is not multiboot"
	exit 1
fi

# Copy the kernel binary and GRUB configuration to the ISO directory
cp "$BUILD_DIR/myos.bin" "$ISO_DIR/boot/myos.bin"
cp "$ISO_DIR/grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"

# Create the ISO image
grub-mkrescue -o "$BUILD_DIR/myos.iso" "$ISO_DIR"

echo "ISO created successfully: $BUILD_DIR/myos.iso"

# start qemu using the binary, bypassing grub.
# -no-reboot: don't reset if the kernel crash
# -d in_asm, int, cpu_reset: Logs every instruction executed by cpu, every interupt and cpu reset events
# -D qemu_instr.log : The log file (redirect -d)
# -serial stdio: redirect COM1 serial port to your terminal

# ===== Pick one of those two
# -kernel "$BUILD_DIR/myos.bin" \
# -cdrom "$BUILD_DIR/myos.iso" \
qemu-system-i386 \
	-cdrom "$BUILD_DIR/myos.iso" \
	-no-reboot \
	"${QEMU_DBG_FLAGS[@]}" \
	-d in_asm,int,cpu_reset \
	-D qemu_instr.log \
	-serial stdio &

# qemu-system-i386 -kernel ./build/myos.bin & # or do this to use the binary directly # -cdrom "$BUILD_DIR/myos.iso" # -kernel "$BUILD_DIR/myos.bin" \

QEMU_PID=$!

# Give QEMU a second to start up
sleep 1

# Launch VNC viewer
vncviewer localhost:5900

# After you close the VNC viewer, kill QEMU
kill $QEMU_PID 2>/dev/null
